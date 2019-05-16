#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int tam = 2;
Semaphore cola[tam] = {1,1};
int n_coches[tam] = {0,0};
mutex mtx;



template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


void  funcion_hebra_coches(  )
{
   int cab = 0;

   while (true)	{
      mtx.lock();
      if(n_coches[0] < n_coches[1]){
         cab = 0;
         n_coches[0]++;
      }
      else{
         cab = 1;
         n_coches[1]++;
      }
      mtx.unlock();

      sem_wait(cola[cab]);
      this_thread::sleep_for( chrono::milliseconds( aleatorio<1000,1500>() ));

      sem_signal(cola[cab]);
      cout << "\nHa pasado un coche por la cabina "<< (cab+1) << endl;
      mtx.lock();
      n_coches[cab]--;
      mtx.unlock();

      this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   }
}

//----------------------------------------------------------------------

int main()
{

   int num_coches = 8;
   thread hebras_coche[num_coches];
   
   for(int i = 0; i < num_coches; i++)
      hebras_coche[i] = thread(funcion_hebra_coches);
   for(int i = 0; i < num_coches; i++)
      hebras_coche[i].join();


}
