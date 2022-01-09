/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
************************************************************************
*
* Test of the Pmod ambient light sensor
*
*************************************************************************
* Description: McLab13_Sensor_SPI_PmodALS_OS6
* The ambient light value will be read and converted to LUX. 
*
* Material
* 1. ST NUCLEO L432KC  or NXP FRDM-K64F  
* or some other micro controller board with SPI communication 
* 2. Digilent Pmod ALS ambient light sensor
* Please connect L432KC or FRDM-K64F - PmodALS with lines:
* L432KC D13 - ALS 4 SCK   hardware defined for the SPI
* L432KC D12 - ALS 3 MISO  hardware defined for the SPI
* L432KC A6  - ALS 1 CS  or any other free
*  GND     - ALS 5 GND
*  Vcc     - ALS 6 Vcc

* ALS data on the SPI
* D15 ... D12 - four zeros
* D11 ... D04 - 8 bits of ambient light data
* D03 ... D00 - four zeros
* 
* Details on the ADC IC on ALS board are given at
* http://www.ti.com/lit/ds/symlink/adc081s021.pdf
* The Pmod ALS 
* https://reference.digilentinc.com/reference/pmod/pmodals/start
*
***************************************************************************
* 3. Option A for task 2: Photo Diode BPW34 and amplifier circuit on HAMK McBoard.
* 3. Option B for task 2: Photo Diode BPW34, OpAmp TLC721, 330k, 12 pF. Detailed 
* circuit diagram in the instruction sheet. 
*
* Timo Karppinen 16.04.2021     copyright Apache-2.0
*************************************************************************/
#include "mbed.h"

DigitalOut LED(D2);  // LD1 to LD4 pin names are linked to D13 in L432KC.
// The L432KC board "LD3" is connected the D13. Do not use! It is the SPI SCK.
// PmodALS
SPI spi(D11, D12, D13); // mosi, miso, sck

DigitalOut alsCS(A6);        // chip select for sensor SPI communication
//DigitalIn sw2(A2);           // input from switch2 on the HAMK McBoard
// Any pin can be used with internal pull down 
DigitalIn sw2(A2, PullDown); // works OK if nothing connected to this pin. 

int alsScaledI = 0;     // 32 bit integer
int getALS();           // function for the Digilent PmodALS sensor
int getPhotoDiode();    // function for the photo diode circuit

int main(){   
// SPI for the ALS        
        // Setup the spi for 8 bit data, high steady state clock,
        // second edge capture, with a 12MHz clock rate
        spi.format(8,0);           
        spi.frequency(2000000); //  1 to 4 MHz recommend for the adc081s021
        // ready to wait the conversion start
        alsCS.write(1);
        ThisThread::sleep_for(1ms);

    while (true) {
     if(sw2.read() == 0){
        alsScaledI = getALS(); 
        }
     else{
        alsScaledI = getPhotoDiode();
        }
          
    printf("Ambient light scaled to LUX =  %0d\r\n",alsScaledI);
        
    if (alsScaledI > 100){ 
        LED.write(1);
        printf("Light enough for working \n\n");
        }
    else{
        LED.write(0);
        printf("Too low light for working \n\n");
        }
            
    ThisThread::sleep_for(3000ms);
    }
}

int getALS(){
    char alsByte0 = 0; //8bit data from sensor board, char is the unsigned 8bit
    char alsByte1 = 0; // 8bit data from sensor board
    char alsByteSh0 = 0;
    char alsByteSh1 = 0;
    char als8bit = 0;    // unsigned 8 bit integer value
    float alsScaledF = 0;       // 32 bit floating point
    
    // Begin the conversion process and serial data output
    alsCS.write(0); 
    ThisThread::sleep_for(1ms);
    // Reading two 8bit bytes by writing two dymmy 8bit bytes
    alsByte0 = spi.write(0x00);
    alsByte1 = spi.write(0x00);
    // End of serial data output and back to tracking mode
    alsCS.write(1);
    ThisThread::sleep_for(1ms);
    // Check the http://www.ti.com/lit/ds/symlink/adc081s021.pdf
    // The data looks like 0000AAAA AAAA0000 on the alsByte0 alsByte1
    printf("alsByte0 alsByte1 in hexadecimal %X %X\r\n",alsByte0, alsByte1); 
    // shifting bits to get the number out
    alsByteSh0 = alsByte0 << 4;
    alsByteSh1 = alsByte1 >> 4;
    
    als8bit =( alsByteSh0 | alsByteSh1 );
    alsScaledF = (float(als8bit))*(float(6.68)); 
    // The value 6.68 is 64 bit double precision floating point of type double.
    // Conversions to 32 bit floating point of type float.
    
    printf("Ambient light ALS sensor 8 bit 0...255 =  '%d' \r\n",als8bit);
    //printf("Ambient light scaled to LUX =  '%0.1f' \r\n",alsScaledF);
    //Sorry, no more float printing in OS6 !
    // But if you really need the floating point printing: 
    // https://forums.mbed.com/t/hitchhikers-guide-to-printf-in-mbed-6/12492
    return (int)alsScaledF; 
}

int getPhotoDiode() {
    // The function for reading the analog input value from
    // the Photo Diode amplifier and scaling it to Lux value.
    AnalogIn ainPD(A0);
    unsigned short pd12bit = 0;
    float pdScaledF = 0.0;
    pd12bit = ainPD.read_u16() >>4;  // leftmost 12 bits moved 4 bits to right.
    printf("Ambient light PD sensor 12 bit 0...4095 =  '%d' \r\n", pd12bit);
    pdScaledF = (float(pd12bit))*(float(0.1));  
    return (int)pdScaledF;
}