# Unique
Redis module. Provide unique queue in the redis.
just like [hashvector](https://github.com/neverlee/hashvector) support int64 vector and float64 vector

## Unique queue
What is the unique queue? A bit like priority queue, you can not put two same key object into the queue. But the order is still FIFO ( first in, first out ). The data structure is like LRU cache.

## API
* unique.pushup key field value
* unique.pushnx key field value
* unique.pushiv key field int64 [int64...]
* unique.pushfv key field float64 [float64...]
* unique.pop key
* unique.popiv key
* unique.popfv key
* unique.getall key
* unique.len key

The int64 vector and float64 vector is not compatible, so you should not use `***iv` and `***fv` api at the same item.

## Exmaple
Compile this module, just run `make`. You can get the src/unique.so.
```
# redis-cli
127.0.0.1:6379> module load /path/to/unique.so
OK
127.0.0.1:6379> unique.pushiv grade andy 1 2
(integer) 1
127.0.0.1:6379> unique.pushiv grade june 3 4 5
(integer) 1
127.0.0.1:6379> unique.pushiv grade andy 3 3 3
(integer) 0
127.0.0.1:6379> unique.popiv grade
1) "andy"
2) (integer) 4
3) (integer) 5
4) (integer) 3
127.0.0.1:6379> unique.popiv grade
1) "june"
2) (integer) 3
3) (integer) 4
4) (integer) 5
```
