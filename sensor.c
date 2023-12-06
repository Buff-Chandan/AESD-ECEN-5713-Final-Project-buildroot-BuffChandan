#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

#define SENSOR  "./sensor.bin"

#define SENSOR_RESOLUTION  16

#define CLEAR_LINE  "\33[2K\r"
#define CLEAR_SCR   ""

const float TEMP_HIGH = 30.00;
const float TEMP_LOW  = 27.50;

enum TEMP_STATE {EXCEED_UPPER, EXCEED_LOWER, WITHIN_RANGE};

double getHumidity(int sensor) {
    uint32_t humid;    
    if(read(sensor, &humid, 3) <= 0)
        return NAN;
    humid &= 0xFFFF;
    return (-6 + (125.0 * humid) / (1UL << SENSOR_RESOLUTION)); 
}

double getTemperature(int sensor) {
    uint32_t temp;
    if(read(sensor, &temp, 3) <= 0)
        return NAN;
    temp &= 0xFFFF;
    return (-46.85 + (175.72 * temp) / (1UL << SENSOR_RESOLUTION));
} 

int main() {
    double temp, humd;
    enum TEMP_STATE tSensor_state = WITHIN_RANGE;

    int fd = open(SENSOR, O_RDWR, S_IRUSR|S_IWUSR);    

    if(fd < 0) {
        perror("Not able to open Sensor Port");
        exit(-1);
    } 

    while(1) {
        temp = getTemperature(fd);
        humd = getHumidity(fd);

        if(isnan(temp) || isnan(humd))
            break;

        printf("Current Temperature : %5.2lf: Current Humidity : %5.2lf\n", temp, humd);
        if((tSensor_state != EXCEED_UPPER) && (temp >= TEMP_HIGH)) {
            printf("ALERT!!!, Temperature exceeds the higher Limit\nTurning ON FAN\n");
            tSensor_state = EXCEED_UPPER;
        }
        else if((tSensor_state != EXCEED_LOWER) && (temp <= TEMP_LOW)) {
            printf("ALERT!!!, Temperature reduced below lower Limit\nTurning OFF FAN\n");
            tSensor_state = EXCEED_LOWER;
        }
        usleep(250000);
    }
    close(fd);
    return 0;
}
