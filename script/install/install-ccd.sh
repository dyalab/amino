#/bin/sh -

wget -q -O - https://github.com/danfis/libccd/archive/v2.0.tar.gz | tar xzf -
cd libccd-2.0
cmake .
make -j"$(nproc)"
make install
