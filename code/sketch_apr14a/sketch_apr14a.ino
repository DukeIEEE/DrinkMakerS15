#define rxPin 19
#define txPin 18


void setup()
{
  Serial1.begin(9600); // setup bluetooth comms
  Serial.begin(9600);// setup serial comms for debugging
  Serial.println("finished init");
  Serial1.println("\n\nfinished");
}

void loop()
{
  if ( Serial1.available() > 0) {
    Serial.println("got stuff");
    Serial1.println("got stuff");
    char inputData = Serial1.read();
    Serial.println(inputData);
  }
  delay(50);
}

