echo Fixing ELF interpreter on $1
patchelf --set-interpreter "$(patchelf --print-interpreter "$(realpath "$(which sh)")")" $1
