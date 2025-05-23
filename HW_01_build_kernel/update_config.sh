#!/bin/bash

CONFIG_FILE=".config"

set_config() {
    local option=$1
    local value=$2

    if grep -q "^$option=" "$CONFIG_FILE"; then
        sed -i "s/^$option=.*/$option=$value/" "$CONFIG_FILE"
    else
        echo "$option=$value" >> "$CONFIG_FILE"
    fi
}

disable_options=(
    CONFIG_BPF
    CONFIG_BPF_SYSCALL
    CONFIG_BPF_JIT
    CONFIG_BPF_EVENTS
    CONFIG_HAVE_EBPF_JIT
    CONFIG_NETFILTER_XT_MATCH_BPF
    CONFIG_TEST_BPF
    CONFIG_BPF_PRELOAD
    CONFIG_BPFILTER
    CONFIG_SECURITY_SELINUX
    CONFIG_SECURITY_SMACK
    CONFIG_SECURITY_TOMOYO
    CONFIG_SECURITY_APPARMOR
    CONFIG_SECURITY_YAMA
    CONFIG_RANDOMIZE_BASE
    CONFIG_CPU_MITIGATIONS
    CONFIG_MITIGATION_SPECTRE_BHI
    CONFIG_MITIGATION_RFDS
    CONFIG_PAGE_TABLE_ISOLATION
    CONFIG_ZSWAP




)

for option in "${disable_options[@]}"; do
    set_config "$option" "n"
done

enable_options=(
    CONFIG_DEBUG_FS
    CONFIG_FTRACE
    CONFIG_FUNCTION_TRACER
    CONFIG_DYNAMIC_FTRACE
    CONFIG_FUNCTION_GRAPH_TRACER
    CONFIG_STACK_TRACER
    CONFIG_KUNIT
    CONFIG_KUNIT_TEST
    CONFIG_KASAN
    CONFIG_STACKTRACE
    CONFIG_KASAN_GENERIC
    CONFIG_KASAN_INLINE
    CONFIG_KASAN_EXTRA_INFO
    CONFIG_KGDB
    CONFIG_KGDB_SERIAL_CONSOLE
    CONFIG_DEBUG_INFO
    CONFIG_SERIAL_CONSOLE
    CONFIG_CONSOLE_POLL
    CONFIG_KPROBES
    CONFIG_KPROBE_EVENT
)

for option in "${enable_options[@]}"; do
    set_config "$option" "y"
done

echo "Config was updated $CONFIG_FILE"
