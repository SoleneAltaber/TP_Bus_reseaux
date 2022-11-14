# TP-Capteurs-et-reseaux
TP ESE de capteurs et réseaux

FRAYSSE Corentin 
ALTABER Solène

**1. TP 1: BUS I²C**

On cherche à piloter le capteur BMP280 DIGITAL PRESSURE SENSOR en I²C.

 - Le protocole I²C

Rappel I²C: C’est un bus série synchrone bidirectionnel half-duplex :</br>
Ainsi, on remarque que le Bus I²C est nécessairement composé de 3 fils : SCL (une clock), SDA (le bus de données) et une Masse.</br>
De plus, il nécessite la présence d’un MASTER qui peut prendre le contrôle du bus et envoyer des requêtes (écriture ou lecture) à des SLAVES (repérable par une “SLAVE Address qui est propre à chaque composants) à travers le bus de données SDA. </br>
SDA possède 6 états possibles: IDLE, START, STOP, 0, 1, ACK (et RESTART et NACK)</br>
On change d’état lors des fronts montants de la clock (SCL) afin d’avoir un protocole synchrone. Le signal SDA est tiré à 1 par une résistance de pull up, les composants (MASTER ou SLAVES) ne peuvent qu’appliquer des 0 ou libérer le bus qui repasse à 1 si aucun autre composant ne le tire à 0.</br>

 - Le capteur

Dans ce TP n°1 nous utiliserons le capteur BMP280 :  DIGITAL PRESSURE SENSOR : </br>
Slave adress : 111011X      → le X est déterminé par le câblage du capteur: si SDO est connecté au GND on aura X=0 si SDO est connecté au VDDIO on aura X=1.</br>
                            → en WRITE : 111011X0 </br>
                            → en READ : 111011X1              ( le dernier Bit indique si le MASTER veut appelle les SLAVE pour une requête de lecture ou écriture)</br>
                            
- Configuration du capteur

Tout d'abord il nous faut savoir quelles valeurs il nous faut mettre dans les registre de configuration:
Registre d'identification du composant : chip_id[7:0] est à l'adresse 0xD0 est contient la valeur 0x58.</br>
Registre déterminant le mode de fonctionnement du capteur : mode[1:0] est à l'adresse 0xF4 est contient la valeur 11 pour est en normal mode.</br>
Registres d'étalonnage du composant: 0xA1 à 0x88. </br>
Registres contenant la température (ainsi que le format): temp_xlsb[7:0] est à l'adresse 0xFC,  temp_lsb[7:0] est à l'adresse 0xFB,   temp_msb[7:0] est à l'adresse 0xFA. </br>
Registres  contenant la pression (ainsi que le format): press_xlsb[7:0] est à l'adresse 0xF9,  press_lsb[7:0] est à l'adresse 0xF8,   press_msb[7:0] est à l'adresse 0xF7. </br>
Define à mettre en début de code: 
######  Code en C
``` C
#define Slave_Add_BMP280 0b1110111
#define ctrl_meas 0xF4
#define status 0xF3
#define ID 0xD0
#define temp_msb 0xFA		//!< 1 octet
#define temp_lsb 0xFB		//!< 1 octet
#define temp_xlsb 0xFC		//!< 4bits
#define timeout 50
#define calib_data
```
On récupère l'ID et les calibration :
######  Code en C
``` C
  if(HAL_OK != HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, ID, 1, &Ident, sizeof(Ident), timeout)){
  	  printf("Error_ID\r\n");
    }
  printf("ID: %d\r\n",Ident);

  if(HAL_OK != HAL_I2C_Mem_Write(&hi2c1, Slave_Add_BMP280 << 1, ctrl_meas, 1, &pData, sizeof(pData), timeout)){
	  printf("Error_ctrl_meas\r\n");
  }
  else {
	  printf("All good, nothing to notice about ctrl_meas\r\n");
  }
  
    if(HAL_OK != HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, cal_data, 2, &cal_data, sizeof(cal_data), timeout)){
   	  printf("Error Calibration transmission\r\n");
   	  HAL_Delay(2000);
     }
  else {
	  printf("All good, nothing to notice about calibration\r\n");
  }
```
On récupère les valeurs de Température:
######  Code en C
``` C
 HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, temp_msb, 1, &adata1, 1, timeout);
 HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, temp_lsb, 1, &adata2, 1, timeout);
 HAL_I2C_Mem_Read(&hi2c1, Slave_Add_BMP280 << 1, temp_xlsb, 1, &adata3, 1, timeout);
```

On trouve dans la datasheet du composant la fonctions permettant le calcul de la température et de la pression compensées, en format entier 32 bits: bmp280_compensate_T_double (BMP280_S32_t adc_T) pour la temperature et bmp280_compensate_P_double (BMP280_S32_p adc_P) pour la pression.</br>
######  Code en C
``` C
int32_t bmp280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) – ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) – ((int32_t)dig_T1)) * ((adc_T>>4) – ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
 ```

------------------------------------
**2. TP 2:  Interfaçage STM32 - Raspberry**

Comment le Raspberry a obtenu son adresse IP? Le reseau est : 192.168.88.0/24 ainsi le reseau peut posséder jusqu'a 2^8 machines. C'est le routeur qui attribut une adresse IP aux RPI


Etapes effectuées au cours du TP 2
- On a flashé la carte à l'aide de RPI-imager
- On s'est connecté au reseau 192.168.88.0/24 en ssh 
- On a obtenu l'adresse IP : 192.168.88.242
- On a modifié les fichiers config.txt et cmdline.txt tel que decrit dans le TP
- On a installé minicom (sudo apt-get install minicom) 
- En rebouclant le RX de la carte son son TX, on a reussi a envoyer et recevoir des chaînes de caractères
- On a connecté de RX de la RPI au TX de la nucléo et inverssement pour pouvoir les faires communiquer l'un avec l'autre
---------------------------------

**TP3 - Interface REST**

On réutilise notre rasberry d'IP=192.168.88.242
On installe pip3 de python3 ainsi que pyserial et flask
On ecrit le fichier hello.py dans lequel on retrouve le code suivant :

###### Code en C:
```C
from flask import Flask   # on import Flask , un framework pour le développement web en Python
app = Flask(__name__)     # on crée l'application
welcome = "Welcome to 3ESE API!"

@app.route('/')           # on défini l'URL (ici on est à la racine) à laquel la fonction va se lancer 
def hello_world():        # on défini une fonction
    return 'Hello, World!\n'    # contenu de la fonction, ici on print Hello world 

@app.route('/api/welcome/') # on défini l'URL (ici on est à la racine/api/welcome) à laquel la fonction va se lancer 
def api_welcome():
    return welcome
    
@app.route('/api/welcome/<int:index>')  # ici le "fragement" <int:index> permet d'utiliser ce morceau d'URL (ici un nombre) comme variable dans la fonction
def api_welcome_index(index):
    return welcome[index]
```    
Une fois le fichier créé, on "run" hello.py avec la commande :
###### Code bash:
```bash
FLASK_APP=hello.py flask run           <- on compile le fichier et on le fait tourner sur notre LocalHost (http://127.0.0.1:5000)
curl http://127.0.0.1:5000             <- il affiche l'entête de la page web d'adresse http://127.0.0.1:5000
```
Pour pouvoir prétendre être RESTful, notre serveur devra: répondre sous forme JSON et différencier les méthodes HTTP.






