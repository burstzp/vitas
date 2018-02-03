**最大的优势就是简单。可用代码不超过1000行。基本上谁都能看懂。**

**缺点是如何备份？下一个版本要考虑的问题**

先装libevent2 和leveldb

./configure

make

make install

vitas -s127.0.0.1:7777 -l vitas.log -p vitas.pid -D test -d

-D 数据存储目录
-d daemon

单进程。

1. memcache协议，支持压缩，支持过期时间设置
2. 轻量，代码简单，容易维护。

