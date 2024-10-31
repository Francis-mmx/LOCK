   _device_node_begin = .;
   PROVIDE(device_node_begin = .);
        *(.device)
   _device_node_end = .;
   PROVIDE(device_node_end = .);
   _audio_output_handler_begin = .;
   PROVIDE(audio_output_handler_begin = .);
        *(.audio_output_handler)
   _audio_output_handler_end = .;
   PROVIDE(audio_output_handler_end = .);

   PROVIDE(audio_input_handler_begin = .);
        *(.audio_input_handler)
   _audio_input_handler_end = .;
   PROVIDE(audio_input_handler_end = .);
