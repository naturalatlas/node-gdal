
#include "sdeerror.h"
#include "cpl_port.h"

CPL_CVSID("$Id: sdeerror.cpp 3b0bbf7a8a012d69a783ee1f9cfeb5c52b370021 2017-06-27 20:57:02Z Even Rouault $")

/************************************************************************/
/*                           IssueSDEError()                            */
/* from ogrsdedatasource.cpp                                            */
/************************************************************************/

void IssueSDEError( int nErrorCode,
                    const char *pszFunction )

{
    char szErrorMsg[SE_MAX_MESSAGE_LENGTH+1];

    if( pszFunction == NULL )
        pszFunction = "SDERASTER";

    SE_error_get_string( nErrorCode, szErrorMsg );

    CPLError( CE_Failure, CPLE_AppDefined,
              "%s: %d/%s",
              pszFunction, nErrorCode, szErrorMsg );
}

/************************************************************************/
/*                           IssueSDEExtendedError()                    */
/************************************************************************/
void IssueSDEExtendedError ( int nErrorCode,
                           const char *pszFunction,
                           SE_CONNECTION* connection,
                           SE_STREAM* stream) {

    SE_ERROR err;
    char szErrorMsg[SE_MAX_MESSAGE_LENGTH+1];

    if( pszFunction == NULL )
        pszFunction = "SDERASTER";

    SE_error_get_string( nErrorCode, szErrorMsg );

    if (connection)
        SE_connection_get_ext_error( *connection, &err );
    if (stream)
        SE_stream_get_ext_error( *stream, &err );

    if (connection || stream) {
        CPLError ( CE_Failure, CPLE_AppDefined,
                   "%s: %d/%s ---- %s ---- %s ---- %s ---- %s",
                   pszFunction, nErrorCode, szErrorMsg,
                   err.sde_error, err.ext_error,
                   err.err_msg1, err.err_msg2 );
    }
    else
    {
        CPLError ( CE_Failure, CPLE_AppDefined,
           "%s: %d/%s",
           pszFunction, nErrorCode, szErrorMsg );
    }
}
