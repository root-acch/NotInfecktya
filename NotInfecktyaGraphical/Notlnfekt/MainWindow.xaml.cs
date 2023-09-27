using Microsoft.Win32.SafeHandles;
using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;




namespace Notlnfekt
{
    public partial class MainWindow : Window
    {
        private Button lastClickedButton = null;
        private Color originalBackgroundColor;
       

        public MainWindow()
        {
            InitializeComponent();
            Main.Content = new Page2 { Background = Brushes.Transparent };
         
        }

        //muda cor dos botoes
        private void ChangeButtonColors(Button clickedButton)
        {
            if (lastClickedButton != null)
            {
                lastClickedButton.Background = new SolidColorBrush(Color.FromRgb(17, 16, 29));
                lastClickedButton.Foreground = new SolidColorBrush(Color.FromRgb(250, 30, 78));
            }

            clickedButton.Background = new SolidColorBrush(Color.FromRgb(250, 30, 78));
            clickedButton.Foreground = new SolidColorBrush(Color.FromRgb(17, 16, 29));
            lastClickedButton = clickedButton;
        }

        private void BtnClickP2(object sender, RoutedEventArgs e)
        {
            Main.NavigationUIVisibility = System.Windows.Navigation.NavigationUIVisibility.Hidden;
            Main.Content = new Page2 { Background = Brushes.Transparent };
            ChangeButtonColors((Button)sender);
            muda_modo.Visibility = Visibility.Collapsed;
        }

        private void BtnClickP3(object sender, RoutedEventArgs e)
        {
            Main.NavigationUIVisibility = System.Windows.Navigation.NavigationUIVisibility.Hidden;
            Main.Content = new Page3 { Background = Brushes.Transparent };
            ChangeButtonColors((Button)sender);
            muda_modo.Visibility = Visibility.Visible;
        }

        private void BtnClickP4(object sender, RoutedEventArgs e)
        {
            Main.NavigationUIVisibility = System.Windows.Navigation.NavigationUIVisibility.Hidden;
            Main.Content = new Page4 { Background = Brushes.Transparent };
            ChangeButtonColors((Button)sender);
            muda_modo.Visibility = Visibility.Collapsed;
        }

        //botão desliga/deixa segundo plano
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }


        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            this.WindowState = WindowState.Minimized;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {                             
            originalBackgroundColor = ((SolidColorBrush)wnd_main.Background).Color;

            if (model_configs.islight)
            {
                var bc = new BrushConverter();
                wnd_main.Background = (Brush)bc.ConvertFrom("#eeeeee");
            }
            else
            {
                var bc = new BrushConverter();
                wnd_main.Background = (Brush)bc.ConvertFrom("#1D1B31");
            }
        }

        private void Image_PreviewMouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            this.DragMove();
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            if (fundo.Background is SolidColorBrush solidColorBrush)
            {
                if (solidColorBrush.Color == Colors.White)
                {
                    fundo.Background = new SolidColorBrush(Color.FromRgb(29, 27, 49));
                    muda_modo.Background = Brushes.White;
                    muda_modo.Foreground = Brushes.Black;
                }
                else
                {
                    fundo.Background = new SolidColorBrush(Colors.White);
                    muda_modo.Background = Brushes.Black;
                    muda_modo.Foreground = Brushes.White;
                }
            }
        }

        
    }
}
