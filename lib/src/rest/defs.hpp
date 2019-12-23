#ifndef REST_DEFS
#define REST_DEFS


#ifndef INCLUDED_BY_REST
// #error Do not include defs.hpp directly
#endif /* INCLUDED_BY_REST */


namespace REST
{
enum class HTTPMethod {
	GET,
	PUT,
	POST,
	DELETE,
	N_SUPPORTED_METHODS,
};

enum class HTTPStatus {
	// TODO (phymod0)
	OK,
	NOT_FOUND,
	TIMEOUT,
};
} // namespace REST


#endif /* REST_DEFS */
