const int LED = 13;

void setup()
{
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
}

void loop()
{
    if (Serial.available())
    {
        char c = Serial.read();

        if (c == '1')
        {
            digitalWrite(LED, HIGH);
        }
        else if (c == '0')
        {
            digitalWrite(LED, LOW);
        }
    }
}
