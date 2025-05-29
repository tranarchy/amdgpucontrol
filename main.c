#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define AMDGPU_PATH "/sys/class/drm/card0/device/"

int apply_conf(char *filename, char *value, int commit) {
    FILE *fp;
    char full_path[256];

    snprintf(full_path, 256, "%s%s", AMDGPU_PATH, filename);

    fp = fopen(full_path, commit ? "a" : "w");

    if (fp == NULL) {
        printf("\nFailed to apply settings\n\n");
        printf("Did you set amdgpu.ppfeaturemask=0xffffffff ?\n\n");
        printf("https://wiki.gentoo.org/wiki/Kernel/Command-line_parameters\n\n");
        return 1;
    }

    fputs(value, fp);
    fclose(fp);

    if (commit) {
        fp = fopen(full_path, "a");
        fputs("c", fp);
        fclose(fp);
    }

    return 0;
}

int read_conf(void) {
    FILE *config_fp;

    char line[128];
    char value_buff[64];

    char performance_level[64];

    int gpu_clock_pstate;
    int gpu_clock_mhz;
    char gpu_pstates[16];

    int vram_pstate;
    int vram_mhz;
    char vram_pstates[16];

    int voltage_offset;

    config_fp = fopen("/etc/amdgpucontrol.conf", "r");

    if (config_fp == NULL) {
        perror("Config file doesn't exist");
        return 1;
    }

    while (fgets(line, sizeof(line), config_fp)) {
        if (sscanf(line, "PERFORMANCE_LEVEL = %s", performance_level) == 1) {
            apply_conf("power_dpm_force_performance_level", performance_level, 0);
        } else if (sscanf(line, "GPU_CLOCK_PSTATE_AND_MHZ = %d %d", &gpu_clock_pstate, &gpu_clock_mhz) == 2) {
            snprintf(value_buff, 64, "s %d %d", gpu_clock_pstate, gpu_clock_mhz);
            apply_conf("pp_od_clk_voltage", value_buff, 1);
        } else if (sscanf(line, "GPU_ENABLED_PSTATES = %[^\n]", gpu_pstates) == 1) {
            apply_conf("pp_dpm_sclk", gpu_pstates, 0);
        } else if (sscanf(line, "VRAM_PSTATE_AND_MHZ = %d %d", &vram_pstate, &vram_mhz) == 2) {
            snprintf(value_buff, 64, "m %d %d", vram_pstate, vram_mhz);
            apply_conf("pp_od_clk_voltage", value_buff, 1);
        } else if (sscanf(line, "VRAM_ENABLED_PSTATES = %[^\n]", vram_pstates) == 1) {
            apply_conf("pp_dpm_mclk", vram_pstates, 0);
        } else if (sscanf(line, "VOLTAGE_OFFSET = %d", &voltage_offset) == 1) {
            snprintf(value_buff, 64, "vo %d", voltage_offset);
            apply_conf("pp_od_clk_voltage", value_buff, 1);
        }
    }

    fclose(config_fp);

    return 0;
}

int main(int argc, char *argv[]) {
    int ret;

    if (geteuid() != 0) {
        puts("You are not root!");
        return 1;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "reset") == 0 || strcmp(argv[1], "r") == 0) {
            apply_conf("pp_od_clk_voltage", "r", 0);

            return 0;
        }   
    }
        
    ret = read_conf();

    return ret;
}
