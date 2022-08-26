namespace CompileConditional
{
    constexpr bool INTERACTIVE { false };
    constexpr bool DEBUG_PRINT { false };
    constexpr bool TEST_ALL    { false };

    constexpr bool REMOVABLE   { true };
}

namespace Board
{
    constexpr uint8_t BLK1 { 1 << 0 };
    constexpr uint8_t RED1 { 1 << 1 };
    constexpr uint8_t ENA1 { 1 << 2 };
    constexpr uint8_t LED1 { 1 << 3 };
    constexpr uint8_t BLK2 { 1 << 4 };
    constexpr uint8_t RED2 { 1 << 5 };
    constexpr uint8_t ENA2 { 1 << 6 };
    constexpr uint8_t LED2 { 1 << 7 };

    constexpr int POSITIONS_PER_BOARD { 8 };
    constexpr int BYTES_PER_BOARD { 4 };
}

namespace Test
{
    constexpr int TUBES { 32 };
    constexpr int BOARDS { TUBES / Board::POSITIONS_PER_BOARD };
}

namespace Teensy
{
    constexpr uint8_t LED1   { 11 };
    constexpr uint8_t DATA1  { 0 };
    constexpr uint8_t SYNC1  { 1 };
    constexpr uint8_t CLOCK1 { 2 };
    constexpr uint8_t CLEAR1 { 3 };

    void enableOutput()
    {
        pinMode(LED1, OUTPUT);
        pinMode(DATA1, OUTPUT);
        pinMode(SYNC1, OUTPUT);
        pinMode(CLOCK1, OUTPUT);
        pinMode(CLEAR1, OUTPUT);
    }

    void setIdleState()
    {
        digitalWrite(LED1, LOW);
        digitalWrite(DATA1, LOW);
        digitalWrite(SYNC1, LOW);
        digitalWrite(CLOCK1, LOW);
        digitalWrite(CLEAR1, LOW);
        digitalWrite(CLEAR1, HIGH);
    }

    void blinkLED(unsigned int time)
    {
        digitalWrite(LED1, HIGH);
        delay(time);
        digitalWrite(LED1, LOW);
        delay(time);
    }

    void initializeForShift()
    {
        digitalWrite(CLEAR1, HIGH);
        digitalWrite(SYNC1, LOW);
        digitalWrite(CLOCK1, LOW);
    }

    void advanceClockByOneTick()
    {
        digitalWrite(CLOCK1, HIGH);
        digitalWrite(CLOCK1, LOW);
    }

    void advanceSyncByOneTick()
    {
        digitalWrite(SYNC1, HIGH);
        digitalWrite(SYNC1, LOW);
    }

    void clearRegisters()
    {
        digitalWrite(CLEAR1, HIGH);
        digitalWrite(CLEAR1, LOW);
        digitalWrite(CLEAR1, HIGH);
    }
}

namespace Sequence
{
    constexpr char ZERO_CHAR { ' ' };    
    constexpr char NULL_CHAR = 0;

    constexpr char START = '{';
    constexpr char STOP  = '}';
    constexpr char DELIM = '|';
    constexpr char RESET = '~';    
}

namespace Mutable
{
    int count { 0 };
    constexpr int BUFFER_SIZE { 4 };
    char sequenceBuffer[BUFFER_SIZE];
}

void resetSequenceBuffer()
{
    Mutable::count = 0;

    for (int i = 0; i < Mutable::BUFFER_SIZE; ++i)
        Mutable::sequenceBuffer[i] = Sequence::NULL_CHAR;
}

uint8_t convertCharToInt(const char input)
{
    if constexpr (CompileConditional::INTERACTIVE)
        return (uint8_t) (input - Sequence::ZERO_CHAR);
    else
        return (uint8_t) input;
}

bool isValidTube(const char input)
{
    uint8_t t = convertCharToInt(input);
    return (t >= 0 && t < Test::TUBES);
}

void parseInput(const char input)
{
    if constexpr (CompileConditional::DEBUG_PRINT)
    {
        Serial.print("[");
        Serial.print(Mutable::count);
        Serial.print("]: ");
        Serial.print(input);
    }
    
    // Just re-naming for ease of typing.
    char* buf = Mutable::sequenceBuffer;

    bool canReceiveBeginChar = (buf[0] == Sequence::NULL_CHAR);
    bool canReceiveFinishChar = (buf[3] == Sequence::NULL_CHAR);
    bool canReceiveTubeChar = (buf[1] == Sequence::NULL_CHAR);
    bool canReceiveModeChar = (buf[2] == Sequence::NULL_CHAR);

    bool shouldReceiveBeginningChar = (Mutable::count == 0);
    bool shouldReceiveEndingChar = (Mutable::count == 3);
    bool shouldReceiveTubeChar = (Mutable::count == 1);
    bool shouldReceiveModeChar = (Mutable::count == 2);

    bool storeStartChar =               \
        canReceiveBeginChar             \
        && shouldReceiveBeginningChar   \
        && input == Sequence::START;

    bool storeFinishChar =              \
        canReceiveFinishChar            \
        && shouldReceiveEndingChar      \
        && input == Sequence::STOP;

    bool storeTubeChar =                \
        canReceiveTubeChar              \
        && shouldReceiveTubeChar        \
        && isValidTube(input);

    bool storeModeChar =                \
        canReceiveModeChar              \
        && shouldReceiveModeChar;

    if (storeStartChar)
    {
        Mutable::sequenceBuffer[Mutable::count++] = input;

        if constexpr (CompileConditional::DEBUG_PRINT)
            Serial.println("\t[START]");
    }
    else if (storeFinishChar)
    {
        Mutable::sequenceBuffer[Mutable::count++] = input;

        if constexpr (CompileConditional::DEBUG_PRINT)
            Serial.println("\t[END]");
    }
    else if (storeTubeChar)
    {
        Mutable::sequenceBuffer[Mutable::count++] = input;

        if constexpr (CompileConditional::DEBUG_PRINT)
        {
            Serial.print("\t[TUBE: ");
            Serial.print(convertCharToInt(input));
            Serial.println("]");
        }
    }
    else if (storeModeChar)
    {
        Mutable::sequenceBuffer[Mutable::count++] = input;

        if constexpr (CompileConditional::DEBUG_PRINT)
        {
            Serial.print("\t[MODE: ");
            Serial.print((bool) convertCharToInt(input));
            Serial.println("]");
        }
    }
    else
    {
        // Do nothing. Input is invalid.
    }

    // All characters in the sequenceBuffer are filled and valid.
    bool canCompleteSequence =                              \
        (Mutable::count > 3)                                \
        && (Mutable::sequenceBuffer[0] == Sequence::START)  \
        && (Mutable::sequenceBuffer[3] == Sequence::STOP);

    if (canCompleteSequence)
    {
        if constexpr (CompileConditional::DEBUG_PRINT)
            Serial.println();

        sendOut();
        resetSequenceBuffer();
    }
}

void sendOut()
{
    int tube = (int) convertCharToInt(Mutable::sequenceBuffer[1]);
    bool mode = (bool) convertCharToInt(Mutable::sequenceBuffer[2]);

    // We need to correct the tube number
    tube = (Test::TUBES - 1) - tube;

    uint8_t control = 0;
    int byteToWriteOn = 0;

    if (tube % 2)
    {
        byteToWriteOn = (tube - 1) / 2;
        control |= Board::ENA2;
        control |= Board::LED2;
        control |= Board::RED2;

        if (!mode)
        {
            control &= ~Board::LED2;
            control &= ~Board::RED2;
            control |= Board::BLK2;
        }
    }
    else
    {
        byteToWriteOn = tube / 2;
        control |= Board::ENA1;
        control |= Board::LED1;
        control |= Board::RED1;

        if (!mode)
        {
            control &= ~Board::LED1;
            control &= ~Board::RED1;
            control |= Board::BLK1;
        }
    }

    constexpr int BYTES = Board::BYTES_PER_BOARD * Test::BOARDS;
    for (int byte = 0; byte < BYTES; ++byte)
    {
        if (byte == byteToWriteOn)
        {
            shiftOut(Teensy::DATA1, Teensy::CLOCK1, LSBFIRST, control);
        }
        else
        {
            shiftOut(Teensy::DATA1, Teensy::CLOCK1, LSBFIRST, 0);
        }
    }
    Teensy::advanceSyncByOneTick();
}


void testAll(bool mode)
{
    char data[] = {
        Sequence::START,
        Sequence::NULL_CHAR,
        Sequence::NULL_CHAR,
        Sequence::STOP
    };

    if (mode)
    {
        data[2] = '#';
    }
    else
    {
        if constexpr (CompileConditional::INTERACTIVE)
            data[2] = Sequence::ZERO_CHAR;
        else
            data[2] = (char) 0;
    }

    for (int i = 0; i < Test::TUBES; ++i)
    {
        if constexpr (CompileConditional::INTERACTIVE)
            data[1] = i + Sequence::ZERO_CHAR;
        else
            data[1] = (char) i;

        for (int j = 0; j < 4; ++j)
            parseInput(data[j]);

        delay(250);
    }
}

void setup()
{
    Teensy::enableOutput();
    Teensy::setIdleState();

    resetSequenceBuffer();

    while (!Serial)
        Teensy::blinkLED(250);
}

void loop()
{
    if constexpr (CompileConditional::TEST_ALL)
    {
        testAll(false);
        testAll(true);
    }
    else
    {
        while (Serial.available() > 0)
            parseInput(Serial.read());
    }
}
