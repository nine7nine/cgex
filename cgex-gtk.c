#include <gtk/gtk.h>

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CGex-Gtk");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Header bar
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "CGex-Gtk");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

    // Back button
    GtkWidget *back_button = gtk_button_new_with_label("Back");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), back_button);

    // Next button
    GtkWidget *next_button = gtk_button_new_with_label("Next");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), next_button);

    // GtkStack
    GtkWidget *stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    // Dummy page
    GtkWidget *page = gtk_label_new("This is a dummy page.");
    gtk_stack_add_named(GTK_STACK(stack), page, "dummy_page");

    gtk_container_add(GTK_CONTAINER(window), stack);
    gtk_widget_show_all(window);

    // Run the GTK main loop
    gtk_main();

    return 0;
}
