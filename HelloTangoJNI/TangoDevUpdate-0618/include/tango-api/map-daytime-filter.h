// Copyright 2013 Motorola Mobility LLC. Part of the Trailmix project.
// CONFIDENTIAL. AUTHORIZED USE ONLY. DO NOT REDISTRIBUTE.
#ifndef TANGO_API_MAP_DAYTIME_FILTER_H_
#define TANGO_API_MAP_DAYTIME_FILTER_H_

#include <cstdint>
#include <string>

namespace tango_api {
/**
 * Find the map of a group of maps with the same name that is closest
 * in daytime (in absolute value) to the given time value.
 *
 * @param query_daytime Daytime in seconds (automatically wraps around at 24h).
 * @param map_name_filter The name of the group of maps.
 * @param storage_location The directory where the maps are stored.
 * @param map_filename The filename to use for maps.
 * @param found_map_path The path to the map that is closest in time to now.
 *
 * @return True if the map name was found, false otherwise.
 */
bool FindBestMapByDaytime(int64_t query_daytime,
                          const std::string& map_name_filter,
                          const std::string& storage_location,
                          const std::string& map_filename,
                          std::string* found_map_path);
}  // namespace tango_api
#endif  // TANGO_API_MAP_DAYTIME_FILTER_H_
