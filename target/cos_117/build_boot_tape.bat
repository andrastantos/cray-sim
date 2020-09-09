set PATH=%PATH%;..\..\simulator\_bin\msvc_x64_Release;..\..\bin
del ..\..\boot_tape.tap
tap_edit ..\..\boot_tape.tap insert iop_overlay1.bin 0  -- insert tapeboot.bin 1  -- insert diskboot.bin 2  -- insert dmp.bin 3  -- ls
