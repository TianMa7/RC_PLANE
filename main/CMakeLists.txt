idf_component_register(
        SRCS
        "RC_PLANE.cpp"
        "lsm6ds3.cpp"
        "mahony_filter.cpp"
        "PID.cpp"
        "controlTasks.cpp"

        REQUIRES
        "esp_driver_i2c")
