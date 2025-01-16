color_map = {
    "#ff8700": "#ffa700",
    "#d97300": "#E69500",
    "#cc6c00": "#cc8500",
    "#bf6500": "#bf7c00",
    "#b35f00": "#b37400",
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