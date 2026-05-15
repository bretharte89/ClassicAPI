function CreateTextureMarkup(file, fileWidth, fileHeight, width, height, left, right, top, bottom, xOffset, yOffset)
    return string.format("|T%s:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d|t",
          file
        , height
        , width
        , xOffset or 0
        , yOffset or 0
        , fileWidth
        , fileHeight
        , left * fileWidth
        , right * fileWidth
        , top * fileHeight
        , bottom * fileHeight
    );
end