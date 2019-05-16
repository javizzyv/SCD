#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


class Recurso : public HoareMonitor
{
    private:
        int contador[2] = {0};
        CondVar cola_espera;
    public:
        void acceder(int tr);
        void fin_acceso(int tr);
        Recurso();
};


Recurso::Recurso()
{
    cola_espera = newCondVar();
}


void Recurso::acceder(int tr)
{   
    if(contador[tr-1] == 2){
        cout << "\nLas copias del recurso " << tr << " están ocupadas actualmante." << endl;
        cola_espera.wait();
        cout << "\nUna hebra esperando por el recurso " << tr << " lo ha cogido." << endl;   
    }

    else
        cout << "\nUna hebra ha cogido el recurso " << tr << endl;
        contador[tr-1]++;  
}


void Recurso::fin_acceso(int tr)
{
    cout << "\nSe ha terminado de usar el recurso " << tr << " así que hay una copia más disponible." << endl;
    cola_espera.signal();
    contador[tr-1]--;
}


void TipoHebra( MRef<Recurso> monitor , int tr)
{
    int tipo_recurso = tr;

    if(tipo_recurso%2 == 0)
        tipo_recurso = 1;
    else
        tipo_recurso = 2;

    while( true ){
        monitor->acceder(tipo_recurso);
        this_thread::sleep_for(chrono::milliseconds(aleatorio<100,200>()));
        monitor->fin_acceso(tipo_recurso);
        this_thread::sleep_for(chrono::milliseconds(aleatorio<100,200>()));
    }        
}


//----------------------------------------------------------------------

int main()
{
    int num_hebras = 8;
    MRef<Recurso> monitor = Create<Recurso>();
    
    thread hebras_recurso[num_hebras];

    for(int i = 0; i < num_hebras; i++)
        hebras_recurso[i] = thread(TipoHebra, monitor, i);
    for(int i = 0; i < num_hebras; i++)
        hebras_recurso[i].join();

}
