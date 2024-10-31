
    _audio_decoder_begin = .;
    PROVIDE(audio_decoder_begin = .);
        *(.audio_decoder)
    _audio_decoder_end = .;
    PROVIDE(audio_decoder_end = .);

    _audio_package_begin = .;
    PROVIDE(audio_package_begin = .);
        *(.audio_package)
    _audio_package_end = .;
    PROVIDE(audio_package_end = .);

