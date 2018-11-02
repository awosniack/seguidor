const int IN1 = 9;
const int IN2 = 7;
const int IN3 = 5; //atençao: NÃO TEM PWM NESSE PINO. se for dar ré, tem que ser full speed.
const int IN4 = 3;

long ultimo_valor=0;
int ultimo_erro = 0;
int sensores[8];
char buffer[20]; //pra debug da leitura dos sensores.


#define Kp 0.1 //esses numeros sao "base" pro pid
#define Kd 2 //tem que testar pra ver qual numeros sao melhores e tals, segui uma "proporção recomendada de 20 pra 1
              //não usei o I porque ele precisa de muito mais processamento e não é muito recomendável aonde o tempo de resposta é pequeno.

#define velocidade_max 200    //definindo uma velocidade limite pro robo
#define velocidade_base 100   //definindo uma velocidade inicial
#define N_SENSORES 8          //numero de sensores

/*
 *        Roda --------------- Roda
 *        (IN4)                (IN1)    --IN3 e IN2 é a rotação inversa de IN4 e IN1 respectivamente
 * 
 * Sensores:7, 6, 5, 4, 3, 2, 1, 0
 * 
 */



void leitura_sensores(int *sensores){//funcao que le os sensores
  sensores[0]=analogRead(A3);
  sensores[1]=analogRead(A2);
  sensores[2]=analogRead(A1);
  sensores[3]=analogRead(A0);
  sensores[4]=analogRead(A10);
  sensores[5]=analogRead(A6);
  sensores[6]=analogRead(A7);
  sensores[7]=analogRead(A8);
}

int posicao(int *sensores,int linha_branca){ //adaptei essa função de uma função presente numa biblioteca QRT SENSORS.
                                             //o argumento linha_branca deve ser 1 se a linha for branca e 0 se for preta em uma superficie branca.
  unsigned char na_linha=0;
  unsigned long media;
  unsigned int soma,valor;

  leitura_sensores(sensores);
  media=0.0;
  soma=0;
  
  for(int i=0;i<N_SENSORES;i++){
  valor = sensores[i];
  if(linha_branca)
    valor = 1000-valor;
  
  if(valor > 200)
    na_linha=1;
  
  if(valor > 50){
    media += (long)(valor)*(i*1000);
    soma += valor;
  }
  
}

if(!na_linha){
  if(ultimo_valor < (N_SENSORES-1)*500)
    return 0;
  else
    return (N_SENSORES-1)*1000;
}
ultimo_valor=media/soma;
return ultimo_valor;

}



void teste_sensores(int *sensores){
  leitura_sensores(sensores);
  for(int i=0;i<8;i++){
    sprintf(buffer, "Sensor %d = %d     ",i,sensores[i]);//serial print não pode imprimir uma string, então taco tudo as coisa que quero imprimir num buffer.
    Serial.print(buffer);}
    sprintf(buffer, "\t");
    Serial.println(buffer);
   delay(200);//delay de 0.2 segundos
}

 
void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.begin(9600);
  delay(5000);
}
void loop() {


//PID

  int pos = posicao(sensores,1);//essa funcao posicao retorna uma "media ponderada" da leitura dos sensores
  Serial.println(pos);

  int erro = pos - 2500; //tem que mecher nesse 2500 aqui pra achar um numero ideal.
  int velocidade_motor = Kp * erro * Kd * (erro - ultimo_erro);
  ultimo_erro = erro;
  int vel_motor_direito = velocidade_base + velocidade_motor;
  int vel_motor_esquerdo= velocidade_base - velocidade_motor;
  
  if(vel_motor_direito > velocidade_max) vel_motor_direito = velocidade_max; // para evitar valores de velocidade mt altos.
  if(vel_motor_esquerdo> velocidade_max) vel_motor_esquerdo= velocidade_max; // para evitar valores de velocidade mt altos.
  if(vel_motor_direito < 0) vel_motor_direito =0; //manter a velocidade positiva.
  if(vel_motor_esquerdo< 0) vel_motor_esquerdo=0; //manter a velocidade positiva.

  analogWrite(IN1,vel_motor_esquerdo);
  analogWrite(IN4,vel_motor_direito);


}
