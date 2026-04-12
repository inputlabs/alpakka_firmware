typedef struct rotation_statevector {
    float ux;
    float uy;
    float uz;
    float phi;
} RotationStateVector;


void update_rotation_state(RotationStateVector *state, const float *gyro, const float *accel, float dt);
void normalize_adjust_state(RotationStateVector *v);