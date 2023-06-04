/**
 * Fast map matching.
 *
 * ubodg_gen command line program configuration
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_FMM_UBODT_CONFIG
#define MM_FMM_UBODT_CONFIG

#include "config/network_config.hpp"
#include "util/cubao_types.hpp"

namespace FMM
{
namespace MM
{
/**
 * Configuration of ubodt_gen command line program
 */
class UBODTGenAppConfig
{
  public:
    /**
     * Constructor of Configuration of ubodt_gen command line program
     * @param argc number of argument
     * @param argv argument data
     */
    UBODTGenAppConfig() {}
    UBODTGenAppConfig(int argc, char **argv);
    /**
     * Load configuration from xml file
     * @param file input xml configuration file
     */
    void load_xml(const std::string &file);
    /**
     * Load configuration from arguments
     * @param argc number of argument
     * @param argv argument data
     */
    void load_arg(int argc, char **argv);
    /**
     * Print information
     */
    void print() const;
    /**
     * Check the validity of the configuration
     * @return true if valid otherwise false
     */
    bool validate() const;
    /**
     * Check if the output is in binary format
     * @return true if binary and otherwise false
     */
    bool is_binary_output() const;
    /**
     * Print help information
     */
    static void print_help();

    // json load/dump
    bool load(const std::string &path);
    bool dump(const std::string &path) const;
    bool loads(const std::string &json);
    std::string dumps() const;
    bool from_json(const RapidjsonValue &json);
    RapidjsonValue to_json(RapidjsonAllocator &allocator) const;
    RapidjsonValue to_json() const
    {
        RapidjsonAllocator allocator;
        return to_json(allocator);
    }

    CONFIG::NetworkConfig network_config; /**< Network configuration */
    double delta;            /**< Upper-bound of the routing result */
    std::string result_file; /**< Result file */
    int log_level = 2;    /**< Level level. 0-trace,1-debug,2-info,3-warn,4-err,
                              5-critical,6-off */
    bool use_omp = false; /**< If true, parallel computing performed */
    bool help_specified = false; /**< Help is specified or not */
};                               // UBODT_Config
} // namespace MM
} // namespace FMM

#endif
