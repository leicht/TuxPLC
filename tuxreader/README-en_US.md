To compile:

    mv ../sutil/lib/libsutil.a /usr/local/lib/
    ln -s /home/leicht/Programmes/fox/tuxplc/sutil/src /usr/local/include/sutil
    mv ../tuxeip/lib/libtuxeip.a /usr/local/lib/
    ln -s /home/leicht/Programmes/fox/tuxplc/tuxeip/src /usr/local/include/tuxeip

To test in console mode:

    echo '[flex]myfloat!' | netcat -q 10 localhost 17560

BUGS:
* When you ask for a tag (tag) that exists, you get a normal response.
Then if we ask for a tag that does not exist with an additional spelling (tag6)
then we get the tag value without error.

* Problem of line return on the display of the list of PLCs.
