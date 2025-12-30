#include <CoreFoundation/CoreFoundation.h>
#include <libproc.h>
#include <signal.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Config {
    std::vector<std::string> process_names = {"WallpaperAerialsExtension"};
    bool verbose = false;
    bool kill_on_unlock = true;
    int primary_signal = SIGTERM;
    int force_signal = SIGKILL;
    int force_after_ms = 0;
};

const char *signal_name(int sig) {
    switch (sig) {
        case SIGTERM:
            return "SIGTERM";
        case SIGKILL:
            return "SIGKILL";
        case SIGINT:
            return "SIGINT";
        default:
            return "SIGNAL";
    }
}

void print_usage(const char *argv0) {
    std::cerr
        << "Usage: " << argv0 << " [options]\n\n"
        << "Options:\n"
        << "  --process <name>           Process name to terminate (repeatable). Default: WallpaperAerialsExtension\n"
        << "  --event <unlock|lock>      Event to trigger on. Default: unlock\n"
        << "  --signal <TERM|KILL>       Primary signal. Default: TERM\n"
        << "  --force-after-ms <ms>      If >0, send --force-signal after this delay. Default: 0 (disabled)\n"
        << "  --force-signal <KILL|TERM> Force signal. Default: KILL\n"
        << "  --verbose                  Print actions to stderr\n"
        << "  -h, --help                 Show help\n";
}

bool kill_targets(const Config &cfg, int sig) {
    int bytes_needed = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
    if (bytes_needed <= 0) {
        if (cfg.verbose) {
            std::cerr << "proc_listpids failed (bytes_needed=" << bytes_needed << ")\n";
        }
        return false;
    }

    std::vector<pid_t> pids(static_cast<size_t>(bytes_needed) / sizeof(pid_t));
    int bytes_filled = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), bytes_needed);
    if (bytes_filled <= 0) {
        if (cfg.verbose) {
            std::cerr << "proc_listpids returned empty list\n";
        }
        return false;
    }

    int pid_count = bytes_filled / static_cast<int>(sizeof(pid_t));

    bool any_signaled = false;
    for (int i = 0; i < pid_count; ++i) {
        pid_t pid = pids[i];
        if (pid == 0) continue;

        char name_buf[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_name(pid, name_buf, sizeof(name_buf)) <= 0) continue;

        std::string proc(name_buf);
        for (const auto &target : cfg.process_names) {
            if (proc != target) continue;

            if (kill(pid, sig) == 0) {
                any_signaled = true;
                if (cfg.verbose) {
                    std::cerr << "Sent " << signal_name(sig) << " to " << proc << " (PID " << pid << ")\n";
                }
            } else if (cfg.verbose) {
                std::cerr << "Failed to send " << signal_name(sig) << " to " << proc << " (PID " << pid
                          << "): " << std::strerror(errno) << "\n";
            }
        }
    }

    return any_signaled;
}

void system_event_callback(CFNotificationCenterRef /*center*/, void *observer, CFStringRef name, const void * /*object*/,
                           CFDictionaryRef /*userInfo*/) {
    auto *cfg = static_cast<Config *>(observer);
    if (!cfg) return;

    const bool is_unlock = CFStringCompare(name, CFSTR("com.apple.screenIsUnlocked"), 0) == kCFCompareEqualTo;
    const bool is_lock = CFStringCompare(name, CFSTR("com.apple.screenIsLocked"), 0) == kCFCompareEqualTo;

    if ((cfg->kill_on_unlock && !is_unlock) || (!cfg->kill_on_unlock && !is_lock)) {
        return;
    }

    bool any = kill_targets(*cfg, cfg->primary_signal);
    if (any && cfg->force_after_ms > 0) {
        usleep(static_cast<useconds_t>(cfg->force_after_ms) * 1000);
        kill_targets(*cfg, cfg->force_signal);
    }
}

bool parse_signal(const std::string &value, int &out) {
    if (value == "TERM" || value == "SIGTERM") {
        out = SIGTERM;
        return true;
    }
    if (value == "KILL" || value == "SIGKILL") {
        out = SIGKILL;
        return true;
    }
    return false;
}

} // namespace

int main(int argc, char **argv) {
    Config cfg;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        if (arg == "--verbose") {
            cfg.verbose = true;
            continue;
        }
        if (arg == "--process" && i + 1 < argc) {
            std::string name(argv[++i]);
            if (cfg.process_names.size() == 1 && cfg.process_names[0] == "WallpaperAerialsExtension") {
                cfg.process_names.clear();
            }
            cfg.process_names.push_back(name);
            continue;
        }
        if (arg == "--event" && i + 1 < argc) {
            std::string ev(argv[++i]);
            if (ev == "unlock") {
                cfg.kill_on_unlock = true;
                continue;
            }
            if (ev == "lock") {
                cfg.kill_on_unlock = false;
                continue;
            }
            std::cerr << "Invalid --event value: " << ev << "\n";
            return 2;
        }
        if (arg == "--signal" && i + 1 < argc) {
            std::string v(argv[++i]);
            if (!parse_signal(v, cfg.primary_signal)) {
                std::cerr << "Invalid --signal value: " << v << "\n";
                return 2;
            }
            continue;
        }
        if (arg == "--force-signal" && i + 1 < argc) {
            std::string v(argv[++i]);
            if (!parse_signal(v, cfg.force_signal)) {
                std::cerr << "Invalid --force-signal value: " << v << "\n";
                return 2;
            }
            continue;
        }
        if (arg == "--force-after-ms" && i + 1 < argc) {
            cfg.force_after_ms = std::stoi(argv[++i]);
            if (cfg.force_after_ms < 0) cfg.force_after_ms = 0;
            continue;
        }

        std::cerr << "Unknown argument: " << arg << "\n";
        print_usage(argv[0]);
        return 2;
    }

    CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
    if (!center) {
        std::cerr << "Failed to get distributed notification center\n";
        return 1;
    }

    CFStringRef event_name = cfg.kill_on_unlock ? CFSTR("com.apple.screenIsUnlocked") : CFSTR("com.apple.screenIsLocked");

    CFNotificationCenterAddObserver(center, &cfg, system_event_callback, event_name, nullptr,
                                    CFNotificationSuspensionBehaviorDeliverImmediately);

    if (cfg.verbose) {
        std::cerr << "Listening for " << (cfg.kill_on_unlock ? "unlock" : "lock") << " events...\n";
    }

    CFRunLoopRun();
    return 0;
}
