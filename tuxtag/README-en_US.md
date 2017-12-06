to compile:

    # mv ../sutil/lib/libsutil.a /usr/local/lib/
    # ln -s /home/leicht/Programs/fox/tuxplc/sutil/src/usr/local/include/sutil
    # mv ../tuxeip/lib/libtuxeip.a /usr/local/lib/
    # ln -s /home/leicht/Programs/fox/tuxplc/tuxeip /src/usr/local/include/tuxeip

Test:

    cat test / test_read.json | netcat -q 10 localhost 17560
