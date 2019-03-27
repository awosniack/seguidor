const int IN1 = 9;
const int IN2 = 7;
const int IN3 = 5; //atençao: NÃO TEM PWM NESSE PINO. se for dar ré, tem que ser full speed.
const int IN4 = 3;

long ultimo_valor=0;
int ultimo_erro = 0;
int sensores[8];
char buffer[20]; //pra debug da leitura dos sensores.


#define Kp 0.4 //provavelmente vai ter que mudar isso conforme aumentar a velocidade do robo
#define Kd 5

#define velocidade_max 250    //definindo uma velocidade limite pro robo
#define velocidade_base 150   //definindo uma velocidade inicial
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



/*
  Retorna uma posição estimada do robo em relação a linha.
  A estimação é calculada usando uma média ponderada dos 
  indices dos sensores multiplicada por 1000, o valor de
  retorno 0 indica que a linha esta abaixo do sensor 0, o
  retorno de 1000 indica que esta abaixo do sensor 1, etc.
  Se a linha estiver entre sensores, a fórmula é:

  0*valor0 + 1000*valor1 + 2000*valor2 + ... + n*1000*valorn
  ----------------------------------------------------------
             valor0 + valor1 + valor2 + ... + valorn
             
*/

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
    if(valor>1000)
      valor = 1000;
    if(linha_branca)
      valor = 1000-valor;

    //Saber se o robo ve a linha
    if(valor > 350) 
      na_linha=1;

    //so calcular a media de alguns valores que passam um certo valor
    if(valor > 200){
      media += (long)(valor)*(i*1000);
      soma += valor;
    }

}

if(!na_linha){//se a ultima leitura foi para a esquerda do sensor retornar 0
  if(ultimo_valor < (N_SENSORES-1)*500)
    return 0;
  else //se foi para a direita, retornar o maximo
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
//   delay(200);//delay de 0.2 segundos
}


int debug_posicao(int *sensores,int linha_branca){ //igual a funcao de posicao, mas imprime no monitor serial quando calcula a media
                                             
  unsigned char na_linha=0;
  unsigned long media;
  unsigned int soma,valor;

  leitura_sensores(sensores);
  media=0.0;
  soma=0;

  for(int i=0;i<N_SENSORES;i++){
    valor = sensores[i];
    if(valor>1000)
      valor = 1000;
    if(linha_branca)
      valor = 1000-valor;

    //Saber se o robo ve a linha
    if(valor > 350) 
      na_linha=1;

    //so calcular a media de alguns valores que passam um certo valor
    if(valor > 200){ 
      sprintf(buffer,"(media)_%ld += %ld * %d\t(soma)_%d += %d",media,(long)valor, i*1000, soma, valor);
      Serial.println(buffer);
      media += (long)(valor)*(i*1000);
      soma += valor;
    }

}

if(!na_linha){//se a ultima leitura foi para a esquerda do sensor retornar 0
  if(ultimo_valor < (N_SENSORES-1)*500)
    return 0;
  else //se foi para a direita, retornar o maximo
    return (N_SENSORES-1)*1000;
}
ultimo_valor=media/soma;
return ultimo_valor;

}

void debug(int *sensores){  //funcao para checar a leitura dos sensores e a velocidade aplicada nos motores.
  for(;;){
//  delay(200);
  int pos = debug_posicao(sensores,1);
  sprintf(buffer, "Ultimo_Valor = %d\t",ultimo_valor);
  Serial.println(buffer);
  teste_sensores(sensores);
  int erro = pos - 3500;
//  int velocidade_motor = Kp * erro; //Sem o Kd
  int velocidade_motor = Kp * erro + Kd * (erro - ultimo_erro);
  ultimo_erro = erro;
  int vel_motor_direito = velocidade_base - velocidade_motor;
  int vel_motor_esquerdo= velocidade_base + velocidade_motor;
  if(vel_motor_direito > velocidade_max) vel_motor_direito = velocidade_max; // para evitar valores de velocidade mt altos.
  if(vel_motor_esquerdo> velocidade_max) vel_motor_esquerdo= velocidade_max; // para evitar valores de velocidade mt altos.
  if(vel_motor_direito < 0) vel_motor_direito =0; //manter a velocidade positiva.
  if(vel_motor_esquerdo< 0) vel_motor_esquerdo=0; //manter a velocidade positiva.

  sprintf(buffer, "Vel_motor_esquerdo = %d               ",vel_motor_esquerdo);
  Serial.println(buffer);
  sprintf(buffer, "Vel_motor_direito = %d \t", vel_motor_direito);
  Serial.println(buffer);
  }

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

  //debug(sensores); //para checar a leitura dos sensores e velocidade nos motores

//PID
//teste_sensores(sensores);

  int pos = posicao(sensores,1);//essa funcao posicao retorna uma "media ponderada" da leitura dos sensores
//  Serial.println(pos);
//  teste_sensores(sensores);
  //delay(400);
  int erro = pos - 3500;
  //Serial.println(erro);
//  int velocidade_motor = Kp * erro * Kd * (erro - ultimo_erro);
  int velocidade_motor = Kp * erro + Kd * (erro - ultimo_erro);
//  int velocidade_motor = Kp * erro;
//  sprintf(buffer, "Erro = %d      Velocidade_motor = %d \t",erro,velocidade_motor);
//  Serial.println(buffer);
  
  //Serial.println(velocidade_motor);
  ultimo_erro = erro;
  int vel_motor_direito = velocidade_base - velocidade_motor;
  int vel_motor_esquerdo= velocidade_base + velocidade_motor;


  if(vel_motor_direito > velocidade_max) vel_motor_direito = velocidade_max; // para evitar valores de velocidade mt altos.
  if(vel_motor_esquerdo> velocidade_max) vel_motor_esquerdo= velocidade_max; // para evitar valores de velocidade mt altos.
  if(vel_motor_direito < 0) vel_motor_direito =0; //manter a velocidade positiva.
  if(vel_motor_esquerdo< 0) vel_motor_esquerdo=0; //manter a velocidade positiva.

//  sprintf(buffer, "Vel_motor_esquerdo= %d \t",vel_motor_esquerdo);
//  Serial.println(buffer);
//  sprintf(buffer, "Vel_motor_direito = %d \t----------------------------------------------------------------------\t",vel_motor_direito);
//  Serial.println(buffer);

   
  analogWrite(IN1,vel_motor_esquerdo);
  analogWrite(IN4,vel_motor_direito);


}
