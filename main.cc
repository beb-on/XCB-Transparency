#include <xcb/xcb.h>
#include <unistd.h>
#include <stdio.h>

int main() {
        xcb_connection_t *connection = xcb_connect(NULL, NULL);

        const xcb_setup_t *setup = xcb_get_setup(connection);
        xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;

        xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen);
        xcb_depth_t *depth = NULL;

        for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
                printf("%i\n", depth_iter.data->depth);
                if (depth_iter.data->depth == 32 && depth_iter.data->visuals_len) {
                        depth = depth_iter.data;
                        break;
                }
        }

        xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth);
        xcb_visualtype_t *visual = NULL;

        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
                if (visual_iter.data->_class == XCB_VISUAL_CLASS_TRUE_COLOR) {
                        visual = visual_iter.data;
                        break;
                }
        }

        xcb_colormap_t colormapId = xcb_generate_id (connection);
        xcb_create_colormap (connection,
                             XCB_COLORMAP_ALLOC_NONE,
                             colormapId,
                             screen->root,
                             visual->visual_id );


        unsigned int cw_mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_COLORMAP;
        unsigned int cw_values[] = { 0x80808080, 0, colormapId };

        xcb_window_t window = xcb_generate_id(connection);
        auto cookie = xcb_create_window_checked(
                connection,
                depth->depth,
                window,
                screen->root,
                0, 0,
                100, 100,
                1,
                XCB_WINDOW_CLASS_INPUT_OUTPUT,
                visual->visual_id,
                cw_mask,
                cw_values);

        auto error = xcb_request_check(connection, cookie);
        if (error) {
                fprintf(stderr, "ERROR: failed to create window: %i\n", error->error_code);
                xcb_disconnect(connection);
                return -1;
        }

        xcb_map_window(connection, window);

        xcb_flush(connection);

        pause();

        xcb_disconnect(connection);
        return 0;
}
