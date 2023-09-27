using System;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using Microsoft.Win32;
using System.Collections.Generic;




namespace Notlnfekt
{
    public partial class Page3 : Page
    {

        public Page3()
        {
            InitializeComponent();
        }


        void change()
        {
            if (model_configs.lang == "en")
            {
                cmb_lung.SelectedIndex = 0;
                lbl_lang.Content = "Language";
                lbl_luz.Content = "Dark/Light Mode";
            }
            else if (model_configs.lang == "pt")
            {
                cmb_lung.SelectedIndex = 1;
                lbl_lang.Content = "Língua";
                lbl_luz.Content = "Modo Escuro/Claro";
            }
            else if (model_configs.lang == "es")
            {
                cmb_lung.SelectedIndex = 2;
                lbl_lang.Content = "Idioma";
                lbl_luz.Content = "Modo Oscuro/Claro";
            }
        }



        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            change();
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
           if(cmb_lung.SelectedIndex == 0)
           {
                model_configs.lang = "en";
              
           }
           else if(cmb_lung.SelectedIndex == 1)
           {
                model_configs.lang = "pt";
           }
           else if (cmb_lung.SelectedIndex == 2)
           {
                model_configs.lang = "es";
           }
            change();
        }
    }

}