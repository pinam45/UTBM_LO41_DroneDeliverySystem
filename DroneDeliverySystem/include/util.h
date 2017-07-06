#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_UTIL_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_UTIL_H


/*-------------------------------------------------------------------------*//**
 * @brief      Check the result of a function, if @c result is lower than zero
 *             print the error message specified by @c format and the variadic
 *             args and the message associated with the errno value then exit
 *             the program
 *
 * @param[in]  result     Function result
 * @param[in]  format     Format string
 * @param[in]  <unnamed>  Args used to replace a format specifier in the format
 *                        string
 */
void check(int result, const char* format, ...);


#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_UTIL_H
