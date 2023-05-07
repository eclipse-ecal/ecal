## Capnp Publisher and Subscriber Example

Minimal Publisher - publishes an example Imu message
`python3 pub.py`

Minimal Subscriber - subscribes to the Imu message
`python3 sub.py`

Capnp Message Struct

```
struct Imu { 
 timestamp @0: Int64; 
 accel @1: List(Float32);
 gyro  @2: List(Float32);  
}
```


