#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

bool esperar = false;

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


class Estanco : public HoareMonitor
{
    private:
        int ingrediente;
        int num_fumadores = 3;
        int num_cigarros = 0;
        CondVar cola_fumadores[3];
        CondVar cola_estanquero;
	    CondVar cola_fumador2;
    public:
        void obtenerIngrediente(int ingr);
        Estanco();
        void ponerIngrediente(int ingr);
        void esperarRecogidaIngrediente();
};



Estanco::Estanco()
{
    ingrediente = -1;
    cola_estanquero = newCondVar();
    cola_fumador2 = newCondVar();

    for(int i = 0; i < num_fumadores ; i++)
        cola_fumadores[i] = newCondVar();
}

void Fumar( int num_fumador )
{

   if(esperar == true && num_fumador == 2){
        cout << "\El fumador 2 ya ha fumado tres veces y decide esperar..." << endl;
        esperar = false;
   }

   else{
        chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

        cout << " Fumador " << num_fumador << "  :"
             << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

        this_thread::sleep_for( duracion_fumar );

        cout << " Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
  }
}


int producirIngrediente()
{
    return aleatorio<0, 2>();
}


void Estanco::obtenerIngrediente(int ingr)
{
    if (ingrediente != ingr)
        cola_fumadores[ingr].wait();

    else if(ingr == 2){
        if(num_cigarros == 4){
            cola_fumador2.signal();
            ingrediente = -1;
            num_cigarros = 0;
            esperar = true;
        }
        else{
            cout << "\nEl fumador " << ingr << " ha cogido un ingrediente" << endl;
            num_cigarros++;
        }
        
        cola_estanquero.signal();
    }

    else
    {
        cout << "\nEl fumador " << ingr << " ha cogido un ingrediente" << endl;
        ingrediente = -1;
        cola_estanquero.signal();
    }
}


void Estanco::ponerIngrediente(int ingr)
{
    ingrediente = ingr;
    cout << "\nEl estanquero ha puesto el ingrediente " << ingr << endl;
    cola_fumadores[ingr].signal();
    if(ingr == 2 && num_cigarros == 4)
        cola_fumador2.wait(); 
}


void Estanco::esperarRecogidaIngrediente()
{
    if(ingrediente != -1){
        cout << "\nEl estanquero está esperando a poner un ingrediente..." << endl;
        cola_estanquero.wait();
    }
}


void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
    while( true ){
        int ingr = producirIngrediente();
        monitor->ponerIngrediente(ingr);
        monitor->esperarRecogidaIngrediente();
    }        
}


void funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
    while( true ){
        monitor->obtenerIngrediente(num_fumador);
        Fumar(num_fumador);
    }    
}

//----------------------------------------------------------------------

int main()
{
    int fumadores = 3;
    MRef<Estanco> monitor = Create<Estanco>();
    
    thread hebra_estanquero(funcion_hebra_estanquero, monitor),
        hebras_fumadores[fumadores];

    for(int i = 0; i < fumadores; i++)
        hebras_fumadores[i] = thread(funcion_hebra_fumador, monitor, i);
    for(int i = 0; i < fumadores; i++)
        hebras_fumadores[i].join();

    hebra_estanquero.join();
}
