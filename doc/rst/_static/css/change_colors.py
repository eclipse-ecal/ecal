import colorsys

if __name__=="__main__":
    # colors_to_replace = [
    #                         "#007bff",
    #                         "#80bdff",
    #                         "#0056b3",
    #                         "#7abaff",
    #                         "#9fcdff",
    #                         "#0069d9",
    #                         "#0062cc",
    #                         "#b3d7ff",
    #                         "rgba(38, 143, 255, .5)",
    #                         "rgba(0, 123, 255, .25)",
    #                         "rgba(0, 123, 255, .5)",
    #                     ]

    # https://convertacolor.com/
    color_map = {
                    "#007bff": "#ffa500",
                    "#80bdff": "#FFD280", 
                    "#0056b3": "#B37400",
                    "#7abaff": "#FFD07A",
                    "#9fcdff": "#FFDD9F",
                    "#0069d9": "#D98C00",
                    "#0062cc": "#CC8400",
                    "#b3d7ff": "#FFE4B3",
                    "#005cbf": "#BF7C00",
                    "rgba(38, 143, 255, .5)": "rgba(255,178,38, .5)",
                    "rgba(0, 123, 255, .25)": "rgba(255, 162, 0, .25)",
                    "rgba(0, 123, 255, .5)": "rgba(255, 162, 0, .5)",
                }
                        

    with open("theme.css", "rb") as fin:
        with open("theme_colorcorrected.css", "wb") as fout:
            for line in fin:
                try:
                    new_line = line.decode('utf-8')
                except UnicodeDecodeError:
                    print ("Decode error")
                    fout.write(line)
                    continue

                for old_color in color_map:
                    new_color = color_map[old_color]
                    new_line = new_line.replace(old_color, new_color)
                fout.write(new_line.encode('utf-8'))