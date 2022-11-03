#Start of the project

SSL:
    version - 3.0.5
steps to setup
    download openssl sources from https://www.openssl.org/source/
    untar tarball
    cd openssl-<version>
    ./Configure
    make
    make test
    make install
    sudo ldconfig /usr/local/lib64/ (path to where the libs of ssl are)
    ln -sf /usr/local/bin/openssl /usr/bin/
