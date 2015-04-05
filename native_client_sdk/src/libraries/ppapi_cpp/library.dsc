{
  'SEARCH': [
      '../../../../ppapi/cpp',
      '../../../../ppapi/cpp/dev',
      '../../../../ppapi/utility',
      '../../../../ppapi/utility/graphics',
      '../../../../ppapi/utility/threading',
      '../../../../ppapi/utility/websocket',
  ],
  'TARGETS': [
    {
      'NAME' : 'ppapi_cpp',
      'TYPE' : 'lib',
      'SOURCES' : [
        # ppapi/cpp
        'array_output.cc',
        'audio.cc',
        'audio_buffer.cc',
        'audio_config.cc',
        'compositor.cc',
        'compositor_layer.cc',
        'core.cc',
        'directory_entry.cc',
        'file_io.cc',
        'file_ref.cc',
        'file_system.cc',
        'fullscreen.cc',
        'graphics_2d.cc',
        'graphics_3d.cc',
        'graphics_3d_client.cc',
        'host_resolver.cc',
        'image_data.cc',
        'input_event.cc',
        'instance.cc',
        'instance_handle.cc',
        'media_stream_audio_track.cc',
        'media_stream_video_track.cc',
        'message_loop.cc',
        'module.cc',
        'mouse_cursor.cc',
        'mouse_lock.cc',
        'net_address.cc',
        'network_list.cc',
        'network_monitor.cc',
        'network_proxy.cc',
        'ppp_entrypoints.cc',
        'rect.cc',
        'resource.cc',
        'tcp_socket.cc',
        'text_input_controller.cc',
        'udp_socket.cc',
        'url_loader.cc',
        'url_request_info.cc',
        'url_response_info.cc',
        'var_array_buffer.cc',
        'var_array.cc',
        'var.cc',
        'var_dictionary.cc',
        'video_decoder.cc',
        'video_encoder.cc',
        'video_frame.cc',
        'view.cc',
        'websocket.cc',

        # ppapi/cpp/dev
        'cursor_control_dev.cc',
        'file_chooser_dev.cc',
        'font_dev.cc',
        'memory_dev.cc',
        'printing_dev.cc',
        'scriptable_object_deprecated.cc',
        'selection_dev.cc',
        'truetype_font_dev.cc',
        'view_dev.cc',
        'zoom_dev.cc',

        # ppapi/utility/graphics
        'paint_aggregator.cc',
        'paint_manager.cc',

        # ppapi/utility/websocket
        'websocket_api.cc',

        # ppapi/utility/threading
        'lock.cc',
        'simple_thread.cc',
      ],
    }
  ],
  'HEADERS': [
    {
      'FILES': [
        'array_output.h',
        'audio_buffer.h',
        'audio_config.h',
        'audio.h',
        'completion_callback.h',
        'compositor.h',
        'compositor_layer.h',
        'core.h',
        'directory_entry.h',
        'file_io.h',
        'file_ref.h',
        'file_system.h',
        'fullscreen.h',
        'graphics_2d.h',
        'graphics_3d_client.h',
        'graphics_3d.h',
        'host_resolver.h',
        'image_data.h',
        'input_event.h',
        'instance.h',
        'instance_handle.h',
        'logging.h',
        'media_stream_audio_track.h',
        'media_stream_video_track.h',
        'message_handler.h',
        'message_loop.h',
        'module_embedder.h',
        'module.h',
        'module_impl.h',
        'mouse_cursor.h',
        'mouse_lock.h',
        'net_address.h',
        'network_list.h',
        'network_monitor.h',
        'network_proxy.h',
        'output_traits.h',
        'pass_ref.h',
        'point.h',
        'rect.h',
        'resource.h',
        'size.h',
        'tcp_socket.h',
        'text_input_controller.h',
        'touch_point.h',
        'udp_socket.h',
        'url_loader.h',
        'url_request_info.h',
        'url_response_info.h',
        'var_array_buffer.h',
        'var_array.h',
        'var_dictionary.h',
        'var.h',
        'video_decoder.h',
        'video_encoder.h',
        'video_frame.h',
        'view.h',
        'websocket.h',
      ],
      'DEST': 'include/ppapi/cpp',
    },
    {
      'FILES': [
        'cursor_control_dev.h',
        'file_chooser_dev.h',
        'font_dev.h',
        'memory_dev.h',
        'printing_dev.h',
        'scriptable_object_deprecated.h',
        'selection_dev.h',
        'truetype_font_dev.h',
        'video_capture_client_dev.h',
        'video_decoder_client_dev.h',
        'view_dev.h',
        'widget_client_dev.h',
        'zoom_dev.h',
      ],
      'DEST': 'include/ppapi/cpp/dev',
    },
    {
      'FILES': [
        'completion_callback_factory.h',
        'completion_callback_factory_thread_traits.h',
      ],
      'DEST': 'include/ppapi/utility',
    },
    {
      'FILES': [
        'paint_aggregator.h',
        'paint_manager.h',
      ],
      'DEST': 'include/ppapi/utility/graphics',
    },
    {
      'FILES': [
        'paint_aggregator.h',
        'paint_manager.h',
      ],
      'DEST': 'include/ppapi/utility/graphics',
    },
    {
      'FILES': [
        'websocket_api.h',
      ],
      'DEST': 'include/ppapi/utility/websocket',
    },
    {
      'FILES': [
        'lock.h',
        'simple_thread.h',
      ],
      'DEST': 'include/ppapi/utility/threading',
    },
  ],
  'DEST': 'src',
  'NAME': 'ppapi_cpp',
}

