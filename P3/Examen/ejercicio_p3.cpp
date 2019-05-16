#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   na = 8 ,
   nc = 1 ,
   id_alumno = 0 ,
   id_camion = 1,
   id_reponedor = 2,
   num_procesos_esperado = na+nc+1 ,
   etiq_una = 3,
   etiq_dos = 4;


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

void funcion_reponedor()
{
   int valor;
   MPI_Status  estado ;

   while(true){
      MPI_Recv ( &valor, 1, MPI_INT, id_camion, 0, MPI_COMM_WORLD,&estado );
      this_thread::sleep_for(chrono::milliseconds(aleatorio<100,200>()));
      cout << "Se han respuesto las hamburguesas." << endl;
      MPI_Ssend( &valor,  1, MPI_INT, id_camion, 0, MPI_COMM_WORLD);
   }
}
// ---------------------------------------------------------------------

void funcion_alumno(int id)
{
    int valor;
    MPI_Status  estado ;

    while(true){
        this_thread::sleep_for(chrono::milliseconds(aleatorio<100,200>()));
        cout << "El alumno " << id << " está yendo al camion." << endl;
        if(id%2==0){
            MPI_Ssend( &valor,  1, MPI_INT, id_camion, etiq_una, MPI_COMM_WORLD);
            cout << "El alumno " << id << " ha pedido una hamburguesa." << endl;
            MPI_Recv ( NULL, 1, MPI_INT, id_camion, 0, MPI_COMM_WORLD,&estado );
            cout << "El alumno " << id << " ha recibido una hamburguesa." << endl;
        }else{
            MPI_Ssend( &valor,  1, MPI_INT, id_camion, etiq_dos, MPI_COMM_WORLD);
            cout << "El alumno " << id << " ha pedido dos hamburguesas." << endl;
            MPI_Recv ( &valor, 1, MPI_INT, id_camion, 0, MPI_COMM_WORLD,&estado );
            cout << "El alumno " << id << " ha recibido dos hamburguesas." << endl;
        } 
    } 
}
// ---------------------------------------------------------------------

void funcion_camion()
{
   int        valor, emisor;    // identificador de emisor aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido
   int num_hamburguesas = 12;

   while(true)
   {
      if ( num_hamburguesas > 4 )               
        MPI_Probe( id_alumno, MPI_ANY_TAG, MPI_COMM_WORLD,&estado );       
      else if(num_hamburguesas > 0)
        MPI_Probe( id_alumno, etiq_una, MPI_COMM_WORLD,&estado );  
      else{
        cout << "No quedan hamburguesas y el dependiente llama al reponedor." << endl;
        MPI_Ssend( &valor,  1, MPI_INT, id_reponedor, 0, MPI_COMM_WORLD);
        MPI_Probe( id_reponedor, MPI_ANY_TAG, MPI_COMM_WORLD,&estado ); 
      }      

      emisor=estado.MPI_SOURCE;
      MPI_Recv ( &valor, 1, MPI_INT, emisor, estado.MPI_TAG, MPI_COMM_WORLD,&estado );

      switch( estado.MPI_SOURCE ) // leer emisor del mensaje en metadatos
      {
         case id_alumno: 
            switch( estado.MPI_TAG ){
                case etiq_una:
                    num_hamburguesas--;
                break;

                case etiq_dos:
                    num_hamburguesas -= 2;
                break;
            }
         break;

         case id_reponedor: 
            num_hamburguesas = 12;
         break;
      }
   }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      if ( id_propio >=0 && id_propio <=7 )
         funcion_alumno(id_propio);
      else if ( id_propio == 8 )
         funcion_camion();
      else
         funcion_reponedor();
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
