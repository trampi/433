// build with g++ wiring.cpp -o wiring -lwiringPi

#include <wiringPi.h>
#include <string>

struct Signal {
    int state;
    int duration;
};

struct SignalSequence {
    Signal signals[4];
    int count;
};

struct SignalSequence signalSync = {
    {
        {HIGH, 275},
        {LOW, 2675}
        // total: 2950
    },
    2
};

struct SignalSequence signalBit0 = {
    {
        {HIGH, 275},
        {LOW, 275},
        {HIGH, 275},
        {LOW, 1225}
        // total: 2050
    },
    4
};

struct SignalSequence signalBit1 = {
    {
        {HIGH, 275},
        {LOW, 1225},
        {HIGH, 275},
        {LOW, 275}
        // total: 2050
    },
    4
};

struct SignalSequence longOff = {
    {
        {LOW, 8000}
    },
    1
};

struct SignalSequence shortHigh = {
    {
        {HIGH, 275}
    },
    1
};

int pin = 0;

void sequenceDelay(int ms) {
    delayMicroseconds(ms);
}

void sendSequence(SignalSequence &sequence) {

    for (int i = 0; i < sequence.count; i++) {
        Signal signal = sequence.signals[i];
        digitalWrite(pin, signal.state);
        sequenceDelay(signal.duration);
    }

}

void printUsage() {
    printf("Usage:\n");
    printf("./apa3-1500r addr devicenr on|off\n");
    printf("\n");
    printf("parameter:\n");
    printf("        addr       The address of your devices in hex, e.g. a654321\n");
    printf("        devicenr   The device you want to toggle, e.g.\n");
    printf("\n");
    printf("e.g.\n");
    printf("./apa3-1500r 9E0DCA 0 off\n");
    printf("./apa3-1500r 9E0DCA all on\n");
}

void send(int bit) {
    if (bit == 0) {
        sendSequence(signalBit0);
    } else if (bit == 1) {
        sendSequence(signalBit1);
    } else {
        printf("wtf");
    }
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        printUsage();
        return 1;
    }

    long addr = strtol(argv[1], NULL, 16);

    int deviceNr;
    std::string deviceNrStr = argv[2];
    if (deviceNrStr != "0" && atoi(argv[2]) == 0) {
        printUsage();
        return 2;
    } else if (deviceNrStr == "all") {
        deviceNr = 0x20;
    } else {
        deviceNr = atoi(argv[2]);
    }

    int state;
    std::string stateStr = argv[3];
    if (stateStr == "on") {
        state = 0x10;
    } else if (stateStr == "off") {
        state = 0;
    } else {
        printUsage();
        return 3;
    }

    wiringPiSetup();
    pinMode(pin, OUTPUT);
    if (piHiPri(99) == -1) {
        printf("setting priority failed, if it does not work reliable then run as root\n");
    }

    int signals = (addr << 6) | deviceNr | state;
    printf("sending: ");
    for(int i = 31; i >= 0; i--) {
        printf("%i", (signals >> i) & 1);
    }
    printf("\n");

    for (int i = 0; i < 5; i++) {
        sendSequence(longOff);
        sendSequence(signalSync);
        for(int bit = 31; bit >= 0; bit--) {
            send((signals >> bit) & 1);
        }
        sendSequence(shortHigh);
        sendSequence(longOff);
    }

    digitalWrite(pin, LOW);

    printf("done\n");
}
