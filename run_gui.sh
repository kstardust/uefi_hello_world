qemu-system-aarch64 \
  -M virt \
  -accel hvf \
  -cpu host \
  -smp 4 \
  -m 1024 \
  -drive file=pflash0.img,format=raw,if=pflash,readonly=on \
  -drive file=pflash1.img,format=raw,if=pflash \
  -device virtio-gpu-pci \
  -drive file=ultimate_hello_aarch64.bin,format=raw 
