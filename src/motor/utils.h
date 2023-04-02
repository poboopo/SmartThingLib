
#define ALPHA 0.4 // коэффициент фильтрации при измерении положения с потенциометра, для AS5600 можно поставить 1.0
#define kP 10.0   // пропорциональный коэффициент PID-регулятора 
#define kI 1.0   // интегральный коэффициент PID-регулятора    
#define kD 1.0   // дифференциальный коэффициент PID-регулятора 

float pid(float input, float trgt, float kp, float ki, float kd, float dt){
  static float integral = 0.f;    // храним значение суммы интегральной компоненты
  static float lastError = 0.f;   // и предыдущую ошибку регулирования для дифференциирования

  if(dt == 0.f){
    integral = 0.f;
    lastError = 0.f; 
    return 0.f;
  }
  
  float error = trgt - input;   // получаем ощибку регулирования
  integral += ki * error * dt;  // пересчитываем интегральную сумму
  float diff = (error - lastError) / dt;  // ищем дифференциал

  lastError = error;
  return kp * error + integral + kd * diff;   // считаем и выводим результат
}

float lpFilter(float value, float oldValue, float alp){
  return oldValue*(1.f-alp) + alp*value;
}