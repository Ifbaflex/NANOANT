#include <Servo.h>
Servo  myservo;

// SETUP DES PINS
//pins des drivers
const int ENA_1 = 2;
const int DIR_1 = 3;
const int PUL_1 = 4;
const int ENA_2 = 5;
const int DIR_2 = 6;
const int PUL_2 = 7;
//pins des limit switch
const int sw_depl = A0;
const int sw_azim = A1;
const int sw_elev = A2;
//pin solenoide
const int SOL = 8;
//LEDs de test
const int led_init = 10;
const int led_depl = 11;
const int led_cal = 12;
const int led_cont = 9;

// constantes de moteur
const int pulse_rev = 800; // number of steps per revolution
const int SPEED = 800; //microseconds between pulses
const float reduction = 5.18;


// SETUP DES VARIABLES INITIALES
// angles
float ang_az; // angle cible
float ang_el; // angle cible
float ref_az; // angle initial az
float ref_el; // angle initial el
float delta_az; // variation d'angle souhaitée
float delta_el; // variation d'angle souhaitée
// controle des moteurs 
float step_az; // nombre de steps à tourner en azimut
float step_el; // nombre de steps à tourner en élévation
float ratio; // ratio des steps utilisé pour controle simultane
float reste; // reste de la division du ratio
char mode = 'I';
char nextmode;
int step_count_az = 0;
int step_count_el = 0;

// flag pour demande de position (a amélioré si temps de libre)
int ANGflag = 0;
int mssAZ = 0;
int mssEL = 0;
int mssC = 0;
int Cflag = 0;
int AZflag = 0;
int ELflag = 0;
int Rmode = 0;
int flag_calib_az = 0;
int flag_calib_el = 0;
int depl_faite = 0; // flag qui vérifie que le deploiement est fait
int calib_faite = 0; // flag qui assure que la calibration a été faite au moins une fois avant d'autoriser le mode RUN
int flagI = 0;

int pos = 0;

void turn_az(){
   digitalWrite(PUL_1,HIGH);
   delayMicroseconds(SPEED);
   digitalWrite(PUL_1,LOW);
   delayMicroseconds(SPEED);  
}

void turn_el(){
   digitalWrite(PUL_2,HIGH);
   delayMicroseconds(SPEED);
   digitalWrite(PUL_2,LOW);
   delayMicroseconds(SPEED);  
}

void setup() {
  // pins des drivers
  pinMode(ENA_1,OUTPUT);
  pinMode(DIR_1,OUTPUT);
  pinMode(PUL_1,OUTPUT);
  pinMode(ENA_2,OUTPUT);
  pinMode(DIR_2,OUTPUT);
  pinMode(PUL_2,OUTPUT);
  //pin du solenoide
  pinMode(SOL,OUTPUT);
  //pins des LEDs
  pinMode(led_init,OUTPUT);
  pinMode(led_depl,OUTPUT);
  pinMode(led_cal,OUTPUT);
  pinMode(led_cont,OUTPUT);
  //pins des switchs
  pinMode(sw_depl,INPUT);
  pinMode(sw_azim,INPUT);
  pinMode(sw_elev,INPUT);

  // fermer les inputs du driver pour commencer
  digitalWrite(ENA_1,LOW);
  digitalWrite(ENA_2,LOW);

  // interface port seriel
  Serial.begin(9600);

  //Servo deploiement 
  myservo.attach(8);  
  myservo.write(0);  
}

void loop() {
  // lecture du port seriel

  digitalWrite(led_cont,LOW);
  digitalWrite(led_depl,LOW);
  digitalWrite(led_cal,LOW);
  digitalWrite(led_init,LOW);
  delay(200);
  
  if (Serial.available()>0){
    mode = Serial.read();
    Serial.println(mode);
  }
  
  switch (mode){
    case 'I':
      digitalWrite(led_cont,LOW);
      digitalWrite(led_depl,LOW);
      digitalWrite(led_cal,LOW);
      digitalWrite(led_init,HIGH);
      delay(200);
      nextmode = 'I';
      if (flagI != 1){
        Serial.println("Selectionner un mode: D, C, R");
        flagI = 1;
      }
      delay(500);
      //Serial.println(analogRead(sw_depl));
      //Serial.println(analogRead(sw_azim));
      //Serial.println(analogRead(sw_elev));
     break;

     case 'D':
        digitalWrite(led_cont,LOW);
        digitalWrite(led_depl,HIGH);
        digitalWrite(led_cal,LOW);
        digitalWrite(led_init,LOW);
        delay(200);
        //debut de la routine de deploiement
        delay(700);
        //digitalWrite(SOL,HIGH);
        
        for(pos = 0; pos <= 180; pos += 1) // goes from 0 degrees to 180 degrees
          { // in steps of 1 degree
            myservo.write(pos); // tell servo to go to position in variable 'pos'
            delay(10); // waits 15ms for the servo to reach the position
          }        

        while (analogRead(sw_depl) < 1000){
        }
        //digitalWrite(SOL,LOW);
        depl_faite = 1;
        Serial.println("Déploiement complété!");
        nextmode = 'I';
     break;

     case 'C':
        if (depl_faite == 0){
          mode = 'I';
          Serial.println("Vous devez effectuer le déploiement avant de calibrer");
        }
        else{
          digitalWrite(led_cont,LOW);
          digitalWrite(led_depl,LOW);
          digitalWrite(led_cal,HIGH);
          digitalWrite(led_init,LOW);
          delay(200);

          // début de la routine de calibration
          // Axe Azimut en premier
         digitalWrite(DIR_1,LOW);
         while (flag_calib_az == 0){
           if (analogRead(sw_azim) > 1000){
             flag_calib_az = 1;
           }
           else{
              turn_az();
            }
         }
          digitalWrite(DIR_1,HIGH); //inverser la direction pour revenir à 0
         step_count_az = round((70.0/3600)*pulse_rev*reduction);
         Serial.println("On revient de 7 deg en azimut: Égal à "+String(step_count_az)+" steps");
         delay(500);
         for (int x=0; x < step_count_az; x++){
            turn_az();
            //Serial.println(x);
         }
         ref_az = 0;
         flag_calib_az = 0;
         //fin de l'axe azimut
          delay(1000);
         // Axe élévation ensuite
         digitalWrite(DIR_2,HIGH);
         while (flag_calib_el == 0){
           if (analogRead(sw_elev) > 1000){
             flag_calib_el = 1;
            }
           else{
             turn_el();
           }
         }
         digitalWrite(DIR_2,LOW); //inverser la direction pour revenir à 0
         step_count_el = round((70.0/3600)*pulse_rev*reduction);
         Serial.println("On revient de 7 deg en elevation: Égal à "+String(step_count_el)+" steps");
         delay(500);
         for (int x=0; x < step_count_el; x++){
           turn_el();
           //Serial.println(x);
         }
         ref_el = 0;
         flag_calib_el = 0;
         // fin de l'axe élévation
  
         // fin de la routine de calibration
         nextmode = 'I';
         calib_faite = 1;
         Serial.println("Calibration complétée!");
        }
      break;

     case 'R':
        if (calib_faite ==0){
          mode = 'I';
          Serial.println("Vous devez effectuer la calibration avant de contrôler");
        }
        digitalWrite(led_cont,HIGH);
        digitalWrite(led_depl,LOW);
        digitalWrite(led_cal,LOW);
        digitalWrite(led_init,LOW);
        delay(200);
        
        while(mode == 'R'){
          digitalWrite(led_cont,HIGH);
          digitalWrite(led_depl,LOW);
          digitalWrite(led_cal,LOW);
          digitalWrite(led_init,LOW);
          delay(50);

          switch(ANGflag){
            case 0: // entree angle az et el 
              while(ELflag != 1){
                while(AZflag != 1){    // en attente des input de position (x: az; y: elev)
                  if (mssAZ != 1){
                    Serial.println("Entrez l'angle en Azimut (Dixièmes de Degrés)");
                    mssAZ = 1;
                  }
                  if (Serial.available()>1 & mssAZ == 1){
                    ang_az = Serial.parseInt(); 
                    ang_az = ang_az/1.7162;
                    Serial.println(ang_az);
                    if (ang_az > 100 || ang_az < -100){
                      Serial.println("Veuillez entrer une valeur entre -100 et 100 dixièmes de degrés");
                      AZflag = 0;
                      mssAZ = 0;
                    }
                    else{
                      AZflag = 1;
                    }
                  }
                } // fin de l'acquisition de l'angle d'azimut (maintenant stockée dans ang_az)
                
                if (mssEL != 1){ // début de la boucle d'acquisition de l'angle d'élévation
                    Serial.println("Entrez l'angle en Elevation (Dixièmes de Degrés)");
                    mssEL = 1;
                }
                if (Serial.available()>1 & mssEL == 1){
                  ang_el = Serial.parseInt(); 
                  ang_el = ang_el/2.0464;
                  Serial.println(ang_el);
                  if (ang_el > 100 || ang_el < -100){
                      Serial.println("Veuillez entrer une valeur entre -100 et 100 dixièmes de degrés");
                      ELflag = 0;
                      mssEL = 0;
                    }
                    else{
                      ELflag = 1;
                      ANGflag = 1 ; // on peut maintenant passer aux commandes moteurs
                    }
                }
              }              
            break;

            case 1: // rotation du moteur en deux axes
              Serial.println("Rotation des moteurs");
              // début du controle des moteurs
              // Delta angles ciblés
              delta_az = ang_az - ref_az;
              delta_el = ang_el - ref_el;
              Serial.println("Cible en azimut: " + String(ang_az));
              Serial.println("Cible en élévation: " + String(ang_el));
              Serial.println("Delta en azimut: " + String(delta_az));
              Serial.println("Delta en élévation: " + String(delta_el));
              // Convertir en nombre de microsteps souhaités
              step_az = round((delta_az/3600)*pulse_rev*reduction);
              step_el = round((delta_el/3600)*pulse_rev*reduction);
              Serial.println("Nombre de steps en Azimut: " + String(step_az));
              Serial.println("Nombre de steps en Élévation: " + String(step_el));
              
              //determiner sens de rotation des moteurs
              if (step_az >= 0){
                digitalWrite(DIR_1, HIGH);
              }
              else{
                digitalWrite(DIR_1, LOW);
              }
              
              if (step_el >= 0){
                digitalWrite(DIR_2, HIGH);
              }
              else{
                digitalWrite(DIR_2, LOW);
              }
              // on remet les steps en valeur abs puisque le sens de rotation est deja etabli par la pin DIR1 et DIR2 du driver
              step_az = abs(step_az);
              step_el = abs(step_el);

              // arithmetique de controle simultané
              step_count_az = 0;
              step_count_el = 0;
              if (step_az >= step_el){
                // il y a plus de rotation en azimut!
                ratio = step_az/step_el;
                //Serial.println(ratio);
                reste = ratio - round(ratio);
                if (reste < 0){
                  reste = 1 + reste; //on veut absolument la plus grande valeur
                }
                //Serial.println(reste);
                for (int x = 0; x < step_az+step_el; x++){
                  //boucle de rotation
                  if ((step_count_el*ratio - step_count_az) > (reste/2)){
                    turn_az();
                    step_count_az = step_count_az+1;
                  }
                  else{
                    turn_el();
                    step_count_el = step_count_el + 1;
                  }
                }
              } // fin de la boucle si Az > EL
              
              else{
                // il y a plus de rotation en élévation
                ratio = abs(step_el/step_az);
                //Serial.println(ratio);
                reste = ratio - round(ratio);
                if (reste < 0){
                  reste = 1 + reste; //on veut absolument la plus grande valeur
                }
                //Serial.println(reste);
                for (int x = 0; x < step_el+step_az; x++){
                  //boucle de rotation
                  if ((step_count_az*ratio - step_count_el) > (reste/2)){
                    turn_el();
                    step_count_el = step_count_el+1;
                  }
                  else{
                    turn_el();
                    step_count_az = step_count_az + 1;
                  }
                }
              } // fin de la boucle si El > Az
              Serial.println("Nombre de steps en Azimut: "+String(step_count_az));
              Serial.println("Nombre de steps en Elevation: "+String(step_count_el));
              // set les references pour la prochaine fois
              ref_az = ang_az;
              ref_el = ang_el;
              // fin du controle des moteurs

              
             // ANGflag = 2;
              ANGflag = 3;
              Cflag = 0;
              
            break;

//            case 2: //rotation du moteur en elevation 
//              Serial.println("gg EL");              
//              ANGflag = 3;
//              Cflag = 0;  // On entre dans la boucle du message "continuer"
//            break;

            case 3:
              while (Cflag !=1){
                if (mssC != 1){
                  Serial.println("Continue ? Y=01 or N=00");
                  mssC = 1;                  
                }          
                if (Serial.available()>1 & mssC == 1){
                  Rmode = Serial.parseInt(); // réponse à la question continuer?
                  Cflag = 1;  // fin de la boucle continuer                
                }                        
              }
              //Serial.println(Cflag);
              //Serial.println(Rmode);
              //Serial.println(ANGflag);
              //Serial.println(mode);
              if (Cflag == 1 & Rmode == 1){ // on veut continuer
                mssAZ = 0;
                mssEL = 0;
                AZflag = 0;
                ELflag = 0;
                ANGflag = 0;
                Cflag = 0;
                mssC = 0;
                mode = 'R';
                //Serial.println("On recomence le controle");              
              }
              if ((Cflag == 1 & Rmode == 0) or (Cflag == 1 & Rmode != 0 & Rmode != 1)){ // on veut quitter
                ANGflag = 0;
                mssAZ = 0;
                mssEL = 0;
                AZflag = 0;
                ELflag = 0;
                ANGflag = 0;
                Cflag = 0;
                mssC = 0;
                mode = 'I';
                nextmode = mode;
                flagI = 0;
                Serial.println("Au revoir");              
              }
            break;
            default:
              mode = 'I';
              nextmode = mode;
              ANGflag = 0;
              //Serial.println("def1");              
            break;
          }
        
        }
      default :
        mode = nextmode;
        //Serial.println("default");
      break;
  }
}
