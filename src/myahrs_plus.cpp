#include "myahrs_plus.hpp"


WithrobotIMU::EulerAngle WithrobotIMU::Quaternion::to_euler_angle() {
    double square_abs_size = x*x+y*y+z*z+w*w;
    double xx = x*x;
    double xy = x*y;
    double xz = x*z;
    double xw = x*w;
    double yy = y*y;
    double yz = y*z;
    double yw = y*w;
    double zz = z*z;
    double zw = z*w;
    double ww = w*w;

    double RAD2DEG = 180/M_PI;
    EulerAngle e;
    double sinr = 2.0*(yz + xw)/square_abs_size;
    double cosr = 1 - 2.0*(xx+yy)/square_abs_size;
    e.roll  = atan2(sinr,cosr )*RAD2DEG;

    double sinp = 2.0 * (yw-xz)/square_abs_size;
    if (fabs(sinp)>= 1)
        e.pitch = copysign(90, sinp); // use 90 degrees if out of range
    else
        e.pitch = asin(sinp)*RAD2DEG;

    double siny = 2.0*(xy + zw)/square_abs_size;
    double cosy = 1-2.0*(yy+zz)/square_abs_size;
    e.yaw   = atan2(siny,cosy)*RAD2DEG;
    return e;
}