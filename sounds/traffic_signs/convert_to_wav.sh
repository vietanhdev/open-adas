for i in *.mp3  
do
    sox "$i" "$(basename -s .mp3 "$i").wav"
done

