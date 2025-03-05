#include elevio.h

typedef struct {
    int currentFloor;           // Nåværende etasje (-1 hvis mellom etasjer)
    MotorDirection direction;   // Nåværende retning
    bool doorOpen;               // 1 = åpen, 0 = lukket
    bool stopButtonPressed;      // 1 = stoppknappen trykket
} Elevator;

typedef struct {
    int orders[N_FLOORS][N_BUTTONS]; // 1 hvis knapp er trykket, 0 ellers
} OrderQueue;

typedef struct {
    Elevator elevator;  
    OrderQueue orderQueue;
} ElevatorSystem;

void elevator_init(ElevatorSystem *sys) {
    sys->elevator.currentFloor = elevio_floorSensor(); 
    sys->elevator.direction = DIRN_STOP;
    sys->elevator.doorOpen = 0;
    sys->elevator.stopButtonPressed = 0;
    
    memset(sys->orderQueue.orders, 0, sizeof(sys->orderQueue.orders));
}

void elevator_updateFloor(ElevatorSystem *sys) {
    int floor = elevio_floorSensor();
    if (floor != -1) {
        sys->elevator.currentFloor = floor;
        elevio_floorIndicator(floor);
    }
}

void elevator_addOrder(ElevatorSystem *sys, int floor, ButtonType button) {
    sys->orderQueue.orders[floor][button] = 1;
    elevio_buttonLamp(floor, button, 1);
}

void elevator_clearOrdersAtFloor(ElevatorSystem *sys, int floor) {
    for (int i = 0; i < N_BUTTONS; i++) {
        sys->orderQueue.orders[floor][i] = 0;
        elevio_buttonLamp(floor, i, 0);
    }
}

void elevator_setMotorDirection(ElevatorSystem *sys, MotorDirection dir) {
    sys->elevator.direction = dir;
    elevio_motorDirection(dir);
}


int elevator_getNextFloor(ElevatorSystem *sys) {
    int currentFloor = sys->elevator.currentFloor;
    MotorDirection dir = sys->elevator.direction;

    // 1. Sjekk bestillinger i nåværende retning
    if (dir == UP) {
        for (int f = currentFloor + 1; f < N_FLOORS; f++) {
            for (int b = 0; b < N_BUTTONS; b++) {
                if (sys->orderQueue.orders[f][b]) {
                    return f;
                }
            }
        }
    } else if (dir == DOWN) {
        for (int f = currentFloor - 1; f >= 0; f--) {
            for (int b = 0; b < N_BUTTONS; b++) {
                if (sys->orderQueue.orders[f][b]) {
                    return f;
                }
            }
        }
    }

    // 2. Ingen bestillinger i retningen → søk i motsatt retning
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (sys->orderQueue.orders[f][b]) {
                return f;
            }
        }
    }

    // 3. Ingen bestillinger igjen → stopp
    return -1;
}

