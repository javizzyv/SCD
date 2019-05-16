// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = (2*num_filosofos)+1 ,
   etiq_pedir    = 0,                                  //etiqueta usada para pedir un tenedor
   etiq_soltar   = 1,                                  //etiqueta usada para soltar un tenedor
   etiq_levantarse= 2,                                 //etiqueta usada cuando un filosofo se levanta de la mesa
   etiq_sentarse = 3;                                  //etiqueta usada cuando un filosofo se sienta en la mesa

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

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % 10, //id. tenedor izq.
      id_ten_der = (id+num_procesos-2) % 10, //id. tenedor der.
      id_camarero= 10;
      MPI_Status estado ;


  while ( true )
  {
    cout <<"Filósofo " <<id << " solicita sentarse en la mesa." <<endl;
    MPI_Ssend( NULL,  0, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);
    cout <<"El filosofo " <<id <<" se sienta en la mesa. " <<endl;

    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    MPI_Ssend( NULL,  0, MPI_INT, id_ten_izq, etiq_pedir, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    MPI_Ssend( NULL,  0, MPI_INT, id_ten_der, etiq_pedir, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    MPI_Ssend( NULL,  0, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    MPI_Ssend( NULL,  0, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD);

    MPI_Ssend( NULL,  0, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);
    cout << "Filosofo " << id << " se levanta de la mesa." << endl;

    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_pedir, MPI_COMM_WORLD,&estado );
     id_filosofo = estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     MPI_Recv ( &valor, 1, MPI_INT, id_filosofo, etiq_soltar, MPI_COMM_WORLD,&estado );
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}

void funcion_camarero()
{
  int id, filosofos_sentados=0 ;  // valor recibido, numero de fisolofos sentados
  MPI_Status estado ;       // metadatos de las dos recepciones
  
  while(true)
  {
    if(filosofos_sentados<4)
      MPI_Probe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,&estado );
    else
      MPI_Probe( MPI_ANY_SOURCE, etiq_levantarse, MPI_COMM_WORLD,&estado );

    id=estado.MPI_SOURCE;

    switch(estado.MPI_TAG)
    {
      case etiq_sentarse:
        MPI_Recv ( NULL, 0, MPI_INT, id, etiq_sentarse, MPI_COMM_WORLD,&estado );
        filosofos_sentados++;
        cout <<"Al filosofo " << id << " se le ha dado permiso para sentarse. (sentados = " << filosofos_sentados << ")" << endl;
      break;
      case etiq_levantarse:
        MPI_Recv ( NULL, 0, MPI_INT, id, etiq_levantarse, MPI_COMM_WORLD,&estado );
        filosofos_sentados--;
        cout << "El camarero le da permiso al filosofo " << id << " para levantarse de la mesa. (sentados = " 
        << filosofos_sentados << ")" << endl;    
      break;
    }
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio == 10)
         funcion_camarero();
      else if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
