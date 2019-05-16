#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

Semaphore ingr_disp[]={0, 0, 0};
Semaphore mostr_vacio = 1;




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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while( true ){
   int i = aleatorio<0, 2>();
   sem_wait( mostr_vacio );
   cout << " El estanquero ha puesto en el mostrador ";
   if(i==0)
      cout << "tabaco.";
   else if(i==1)
      cout << "papel.";
   else
      cout << "una cerilla.";

   sem_signal( ingr_disp[i] );
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << " Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << " Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      sem_wait( ingr_disp[num_fumador] );
      if(num_fumador==0)
         cout << " Se ha retirado el tabaco.";
      else if(num_fumador==1)
         cout << " Se ha retirado el papel.";
      else
         cout << " Se ha retirado la cerilla.";
      sem_signal( mostr_vacio );
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   int num_fumadores = 3;
   thread hebra_estanquero(funcion_hebra_estanquero),
          hebras_fumadores[num_fumadores];

   for(int i = 0; i < num_fumadores; i++)
      hebras_fumadores[i] = thread(funcion_hebra_fumador, i);
   for(int i = 0; i < num_fumadores; i++)
      hebras_fumadores[i].join();
  
   // declarar hebras y ponerlas en marcha
   // ......
}
