cmd_/root/os3/modify/modify_syscall.ko := ld -r -EL  -maarch64linux -T ./scripts/module-common.lds -T ./arch/arm64/kernel/module.lds  --build-id  -o /root/os3/modify/modify_syscall.ko /root/os3/modify/modify_syscall.o /root/os3/modify/modify_syscall.mod.o ;  true
