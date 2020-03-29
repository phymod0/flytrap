#include "../../../utils/macros.h"
#include "../../rest.h"

#include <event2/buffer.h>
#include <event2/http.h>


const HTTPHandler interaction_handlers[] = {
    REST_END_HANDLERS,
};
