#!/usr/bin/env bash
set -e

print_color() {
    # 1. Define local color map inside the function to keep the global scope clean
    local -A colors=(
        # Regular
        [black]='\e[0;30m'     [red]='\e[0;31m'       [green]='\e[0;32m'     [yellow]='\e[0;33m'
        [blue]='\e[0;34m'      [purple]='\e[0;35m'    [cyan]='\e[0;36m'      [white]='\e[0;37m'
        # Bold
        [b_black]='\e[1;30m'   [b_red]='\e[1;31m'     [b_green]='\e[1;32m'   [b_yellow]='\e[1;33m'
        [b_blue]='\e[1;34m'    [b_purple]='\e[1;35m'  [b_cyan]='\e[1;36m'    [b_white]='\e[1;37m'
        # High Intensity
        [i_black]='\e[0;90m'   [i_red]='\e[0;91m'     [i_green]='\e[0;92m'   [i_yellow]='\e[0;93m'
        [i_blue]='\e[0;94m'    [i_purple]='\e[0;95m'  [i_cyan]='\e[0;96m'    [i_white]='\e[0;97m'
        # Backgrounds
        [bg_red]='\e[41m'      [bg_green]='\e[42m'    [bg_yellow]='\e[43m'   [bg_blue]='\e[44m'
    )
    local reset='\e[0m'

    # 2. Extract arguments
    local color_choice="${1,,}" # Convert input to lowercase to prevent casing errors
    shift                      # Shift arguments so "$*" represents only the text string

    # 3. Determine target color string
    local color_code="${colors[$color_choice]}"

    # 4. Print text (fallback to uncolored if color code does not exist)
    if [[ -n "$color_code" ]]; then
        echo -e "${color_code}$*${reset}"
    else
        # If the first argument wasn't a valid color, include it as part of the printed text string
        if [[ -n "$color_choice" ]]; then
            echo "$color_choice $*"
        else
            echo "$*"
        fi
    fi
}

