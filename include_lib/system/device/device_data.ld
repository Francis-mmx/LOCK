
	_video_subdev_begin = .;
	PROVIDE(video_subdev_begin = .);
	 	*(.video_subdev.0)
	 	*(.video_subdev.1)
	 	*(.video_subdev.2)
	 	*(.video_subdev.3)
	 	*(.video_subdev.4)
	 	*(.video_subdev.5)
	_video_subdev_end = .;
	PROVIDE(video_subdev_end = .);

	_audio_subdev_begin = .;
	PROVIDE(audio_subdev_begin = .);
	 	*(.audio_subdev.0)
	 	*(.audio_subdev.1)
	 	*(.audio_subdev.2)
	 	*(.audio_subdev.3)
	_audio_subdev_end = .;
	PROVIDE(audio_subdev_end = .);

	_iic_device_begin = .;
	PROVIDE(iic_device_begin = .);
	 	*(.iic)
	_iic_device_end = .;
	PROVIDE(iic_device_end = .);


        _avin_spi_device_begin = .;
		PROVIDE(avin_spi_device_begin = .);
			*(.sw_spi)
		_avin_spi_device_end = .;
		PROVIDE(avin_spi_device_end = .);

	_video_dev_begin = .;
	PROVIDE(video_dev_begin = .);
	 	*(.video_device)
	_video_dev_end = .;
	PROVIDE(video_dev_end = .);

	_uart_device_begin = .;
	PROVIDE(uart_device_begin = .);
	 	*(.uart)
	_uart_device_end = .;
	PROVIDE(uart_device_end = .);
