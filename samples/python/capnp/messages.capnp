@0xb6249c5df1f7e512; 

struct Imu { 
 timestamp @0: Int64; 
 accel @1: List(Float32);
 gyro  @2: List(Float32);  
}

