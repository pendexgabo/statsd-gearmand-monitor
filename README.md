statsd-gearmand-monitor
=======================

A process that monitors and send stats about gearmans jobs and workets to statsd daemon


Options
-------

```
-N <metric>         metric prefix to update (required)
-h <host>           statsd host (default: 127.0.0.1)
-p <port>           statsd port (default: 8125)
-H <host>           gearman host (default: 127.0.0.1)
-P <port>           gearman port (default: 4730)
-t <time>           polling rate (seconds) (default: 2)
-f                  run in foreground mode
-d                  debug mode
```



License
-------

Copyright (c) 2013 Gabriel Sosa

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 