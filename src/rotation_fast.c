#include "rotation_fast.h"
#include <math.h>

// Time constant for the comlementary filter.
// Tune the denominator to adjust the filter's response time.
// A value of 0.8 means the filter will converge in about 3x0.8 = 2.4 seconds.
float comp_tau = 1/0.8f;


void update_rotation_state(RotationStateVector *state, const float *gyro, const float *accel, float dt)
{   
    /* This is a rotation scheme that tracks the orientation of the 'up' unit vector,
    and a rotation angle around the vertical axis (yaw).
    The tracking is done from the local coordinate system, which is aligned with the device's orientation.
    At each step the 'up' vector gets adjusted using an approximate rotation from gyro rates,
    using cross-product with the current 'up' vector.*/
    float gyro_x = gyro[0];
    float gyro_y = gyro[1];
    float gyro_z = gyro[2];
    // yaw is by definition the rotation around the vertical axis,
    // so we can calculate it by projecting the gyro vector onto the current 'up' vector.
    // The 'up' vector is represented by the state vector (ux, uy, uz).
    float yaw_value = state->ux*gyro_x + state->uy*gyro_y + state->uz*gyro_z;
    

    float accel_norm = sqrtf(accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2]);
    // Cross product of the gyro vector with the current 'up' vector gives us the rotation vector
    // that we need to apply to the 'up' vector to adjust it.
    float wcross_vector[3] = {gyro_z*state->uy - gyro_y*state->uz,
                              gyro_x*state->uz - gyro_z*state->ux,
                              gyro_y*state->ux - gyro_x*state->uy};
    state->phi += yaw_value * dt;

    state->ux += wcross_vector[0] * dt;
    state->uy += wcross_vector[1] * dt;
    state->uz += wcross_vector[2] * dt;

    // Adjusting the 'up' vector using the accelerometer data and complementary filter.
    // Only perform the adjustment if the accelerometer data is within a reasonable range.
    if (accel_norm>8 && accel_norm<12) {
        state->ux += (accel[0]/accel_norm-state->ux) * comp_tau * dt;
        state->uy += (accel[1]/accel_norm-state->uy) * comp_tau * dt;
        state->uz += (accel[2]/accel_norm-state->uz) * comp_tau * dt;
    }
    normalize_adjust_state(state);
}

void normalize_adjust_state(RotationStateVector *v)
{
    /* A normalization method based on successive adjustments, to avoid square root operation*/
    float norm = v->ux * v->ux + v->uy * v->uy + v->uz * v->uz;
    float norm_diff = 1.0f - norm;
    float gain = 1.0f;
    if (fabsf(norm_diff) > 1e-12)
    {
        gain = 1.0f+ norm_diff*0.5f;
        if (gain < 0.01f) gain = 0.01f;
        v->ux *= gain;
        v->uy *= gain;
        v->uz *= gain;
    } 
}