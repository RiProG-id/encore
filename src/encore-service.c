#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    char mode_apply[128];
    char gamestart[128];
    char screenoff[128];
    char command[256];
    FILE *pid_list;

    snprintf(command, sizeof(command), "su -lp 2000 -c \"cmd notification post -S bigtext -t 'ENCORE' \\\"Tag%d Tweaks applied successfully\\\"\"", (int)time(NULL));
    system(command);

    system("mkdir -p /dev/encore/AI");

    FILE *mode_file = fopen("/dev/encore/AI/mode", "w");
    if (mode_file) {
        fprintf(mode_file, "unset");
        fclose(mode_file);
    }

    while (1) {
        sleep(1);

        mode_file = fopen("/dev/encore/AI/mode", "r");
        if (mode_file) {
            fscanf(mode_file, "%s", mode_apply);
            fclose(mode_file);
        }

        sleep(1);

        FILE *window_dump = popen("dumpsys window | grep -E 'mCurrentFocus|mFocusedApp' | grep -Eo \"$(cat /data/encore/gamelist.txt)\" | tail -n 1", "r");
        if (window_dump) {
            while (fgets(gamestart, sizeof(gamestart), window_dump)) {
                gamestart[strcspn(gamestart, "\n")] = 0;
                if (strcmp(mode_apply, "performance") != 0) {
                    snprintf(command, sizeof(command), "am start -a android.intent.action.MAIN -e toasttext \"Boosting game %s\" -n bellavita.toast/.MainActivity", gamestart);
                    system(command);

                    sprintf(command, "pgrep -f %s", gamestart);
                    pid_list = popen(command, "r");
                    if (pid_list) {
                        char pid_buf[128];
                        while (fgets(pid_buf, sizeof(pid_buf), pid_list)) {
                            int pid = atoi(pid_buf);
                            if (pid > 0) {
                                // renice
                                snprintf(command, sizeof(command), "renice -n -20 -p %d", pid);
                                system(command);

                                // ionice
                                snprintf(command, sizeof(command), "ionice -c 1 -n 0 -p %d", pid);
                                system(command);

                                // chrt
                                snprintf(command, sizeof(command), "chrt -f -p 98 %d", pid);
                                system(command);
                            }
                        }
                        pclose(pid_list);
                    }

                    system("sh /system/bin/encore-performance");

                    mode_file = fopen("/dev/encore/AI/mode", "w");
                    if (mode_file) {
                        fprintf(mode_file, "performance");
                        fclose(mode_file);
                    }
                }
            }
            pclose(window_dump);
        }

        sleep(1);

        FILE *screen_dump = popen("dumpsys display | grep 'mScreenState' | awk -F'=' '{print $2}'", "r");
        if (screen_dump) {
            while (fgets(screenoff, sizeof(screenoff), screen_dump)) {
                screenoff[strcspn(screenoff, "\n")] = 0;
                if (strcmp(screenoff, "off") == 0 && strcmp(mode_apply, "sleep") != 0) {
                    system("sh /system/bin/encore-powersave");

                    mode_file = fopen("/dev/encore/AI/mode", "w");
                    if (mode_file) {
                        fprintf(mode_file, "lowpower");
                        fclose(mode_file);
                    }
                }
            }
            pclose(screen_dump);
        }

        if (strcmp(mode_apply, "normal") != 0) {
            system("sh /system/bin/encore-normal");

            mode_file = fopen("/dev/encore/AI/mode", "w");
            if (mode_file) {
                fprintf(mode_file, "normal");
                fclose(mode_file);
            }
        }

        sleep(12);
    }

    return 0;
}