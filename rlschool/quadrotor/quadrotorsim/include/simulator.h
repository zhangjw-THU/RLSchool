//   Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef QUADROTOR_URANUS_SIMULATOR_H
#define QUADROTOR_URANUS_SIMULATOR_H
#include <Eigen/Dense>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <pybind11/pybind11.h>
#include <random>
#include <semaphore.h>
#include <string>

namespace py = pybind11;

namespace quadrotorsim {
const double PI = 3.1415926;
const double GRAVITY_ACCELERATION = 9.80;

typedef struct {
    double g_x;   // global position x;
    double g_y;   // global position x;
    double g_z;   // global position x;
    double b_v_x; // body velocity x;
    double b_v_y; // body velocity y;
    double b_v_z; // body velocity z;
    double g_v_x; // global velocity x;
    double g_v_y; // global velocity y;
    double g_v_z; // global velocity z;
    double w_x;   // body rotation angular speed x;
    double w_y;   // body rotation angular speed y;
    double w_z;   // body rotation angular speed z;
    double roll;
    double pitch;
    double yaw;
    double power;
} PrintableState;

typedef struct {
    Eigen::Vector3f global_position = Eigen::Vector3f(0.0, 0.0, 0.0);
    Eigen::Vector3f global_velocity = Eigen::Vector3f(0.0, 0.0, 0.0);
    Eigen::Vector3f body_angular_velocity = Eigen::Vector3f(0.0, 0.0, 0.0);
    Eigen::Vector4f propeller_angular_velocity =
        Eigen::Vector4f(0.0, 0.0, 0.0, 0.0);
    Eigen::Matrix3f rotation_matrix = Eigen::Matrix3f::Identity();
    double power = 0.0;
} State;

typedef struct {
    Eigen::Vector3f imu_acc;
    Eigen::Vector3f imu_gyro;
    Eigen::Vector3f vio;
} SensorOutput;

// Control Command
typedef Eigen::Vector4f ActionU;

// A default state
const State ZERO_STATE;

// Simulators
class Simulator {
  public:
    Simulator();
    ~Simulator();

    // functions for config
    int get_config(const char *filename);
    void py_get_config(py::str filename);

    // functions for reset
    void reset();
    void reset(State &state);
    void reset(double g_x, double g_y, double g_z, double g_v_x, double g_v_y,
               double g_v_z, double w_x, double w_y, double w_z, double roll,
               double pitch, double yaw);
    void py_reset(py::kwargs kwargs);

    // functions for state
    void get_printable_state(PrintableState &state);
    py::dict py_get_state();

    void read_sensor(SensorOutput &output);
    py::dict py_get_sensor();

    // functions to run simulation
    int run_time(ActionU &U_input, double dt);
    void py_step(py::list act, float dt);

    int check_failure();

    // functions for high-level tasks
    py::list define_velocity_control_task(double dt, int nt, int seed);

  private:
    int run_internal(ActionU &U_input);

    void save_state(State &state, Eigen::Vector3f &body_acc);
    void restore_state(State &state, Eigen::Vector3f &body_acc);

    double _precision;
    bool _configured;

    State _state;
    Eigen::Vector3f _body_acceleration;
    Eigen::Matrix3f _coordinate_converter_to_world;
    Eigen::Matrix3f _coordinate_converter_to_body;

    // parameters for high-level tasks
    Eigen::Vector3f _expected_next_global_velocity;

    // parameters relative to airplane body
    double _quality;
    Eigen::Matrix3f _inverse_inertia;
    Eigen::Matrix3f _drag_coeff_momentum;
    Eigen::Matrix3f _drag_coeff_force;
    Eigen::Vector3f _gravity_center;

    // each row: a 3-dimensional coordinate of propellar center
    // relative to body center in body coordinate system
    Eigen::Matrix<float, 4, 3> _propeller_coord;

    // parameters relative to the thrust
    double _thrust_coeff_ct_0;
    double _thrust_coeff_ct_1;
    double _thrust_coeff_ct_2;
    double _thrust_coeff_ra;
    double _thrust_coeff_mm;
    double _thrust_coeff_jm;
    double _thrust_coeff_phi;

    // parameter for checking failures
    double _fail_max_velocity;
    double _fail_max_angular_velocity;
    double _fail_max_range;

    // other configurations
    double _max_voltage;
    double _min_voltage;
};
}; // namespace quadrotorsim
#endif
