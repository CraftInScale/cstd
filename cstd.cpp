#include "cstd.h"

#include "log/Logger.h"
#include "error.h"

int cstd_init(Logger& global_logger)
{
	int error;
	error = init_global_logger(global_logger);
	if(error != E_OK)
		return error;

	error = create_vec(cstd_error_providers, sizeof(ErrorProviderFn));
	if (error != E_OK)
	{
		destroy_global_logger();
		return error;
	}

	return E_OK;
}

void cstd_deinit()
{
	destroy_vec(cstd_error_providers);
	destroy_global_logger();
}
