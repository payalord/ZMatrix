object zmxCommonConfigForm: TzmxCommonConfigForm
  Left = 195
  Top = 52
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'ZMatrix Visualization Configuration...'
  ClientHeight = 635
  ClientWidth = 634
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Icon.Data = {
    0000010001002020100000000000E80200001600000028000000200000004000
    0000010004000000000080020000000000000000000000000000000000000000
    0000000080000080000000808000800000008000800080800000C0C0C0008080
    80000000FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000002000000000000000000000000000000022222000000000000000000
    0000000000000200000000000000000000000000000000000000000000000000
    0000000000222000000000000000000000000000002020000000000000000000
    0000000000222000000000000000000000000000000000000000000000000000
    000000000002000000000000000000222222222000020000222202002220002A
    AAAAAA20000200002AA22A22AA20002AA2222220020000002AA2AAA2AA200002
    AA200000022222002AA2AAA2AA2000002AA00000000002002AA2A2A2AA200000
    2AA20000000002002AA2A2A2AA20000002AA2000000000002AA2A2A2AA200000
    00AA2200022220202AAAA2AAAA200000002AA220020020002AAAA2AAAA200022
    2222AA20020020202AAA222AAA20002AAAAAAA20000000002AAA202AAA200022
    2222222000020000222220222220000000000000002020000000000000000000
    0000000002000200000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000020002000000000000000000
    0000000000202000000000000000000000000000000200000000000000000000
    000000000000000000000000000000000000000000000000000000000000C000
    0003800000010000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000080000001C0000003}
  OldCreateOrder = False
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 13
  object BaseColorScaleGroupBox: TGroupBox
    Left = 16
    Top = 24
    Width = 289
    Height = 297
    Caption = 'Base Color Scale:'
    TabOrder = 0
    object BaseColorRedScaleGroupBox: TGroupBox
      Left = 8
      Top = 24
      Width = 273
      Height = 73
      Caption = 'Red Scale:'
      TabOrder = 0
      object BaseColorRedScaleTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 1000
        Orientation = trHorizontal
        PageSize = 50
        Frequency = 50
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = BaseColorRedScaleTrackBarChange
      end
      object BaseColorRedScaleWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = BaseColorRedScaleWidgetChange
      end
      object BaseColorRedScaleWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = BaseColorRedScaleWidget
        Min = 0
        Max = 1000
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
    object BaseColorGreenScaleGroupBox: TGroupBox
      Left = 8
      Top = 112
      Width = 273
      Height = 73
      Caption = 'Green Scale:'
      TabOrder = 1
      object BaseColorGreenScaleTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 1000
        Orientation = trHorizontal
        PageSize = 50
        Frequency = 50
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = BaseColorGreenScaleTrackBarChange
      end
      object BaseColorGreenScaleWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = BaseColorGreenScaleWidgetChange
      end
      object BaseColorGreenScaleWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = BaseColorGreenScaleWidget
        Min = 0
        Max = 1000
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
    object BaseColorBlueScaleGroupBox: TGroupBox
      Left = 8
      Top = 200
      Width = 273
      Height = 73
      Caption = 'Blue Scale:'
      TabOrder = 2
      object BaseColorBlueScaleTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 1000
        Orientation = trHorizontal
        PageSize = 50
        Frequency = 50
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = BaseColorBlueScaleTrackBarChange
      end
      object BaseColorBlueScaleWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = BaseColorBlueScaleWidgetChange
      end
      object BaseColorBlueScaleWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = BaseColorBlueScaleWidget
        Min = 0
        Max = 1000
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
  end
  object BaseColorOffsetGroupBox: TGroupBox
    Left = 16
    Top = 344
    Width = 289
    Height = 105
    Caption = 'Base Color Offset:'
    TabOrder = 1
    object BaseColorOffsetColorContainerPanel: TPanel
      Left = 16
      Top = 24
      Width = 73
      Height = 73
      TabOrder = 0
      object BaseColorOffsetColorPanel: TPanel
        Left = 1
        Top = 1
        Width = 71
        Height = 71
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
      end
    end
    object BaseColorOffsetSelectButton: TButton
      Left = 112
      Top = 48
      Width = 75
      Height = 25
      Caption = 'Select...'
      TabOrder = 1
      OnClick = BaseColorOffsetSelectButtonClick
    end
  end
  object ButtonPanel: TPanel
    Left = 0
    Top = 594
    Width = 634
    Height = 41
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 2
    object OKButton: TButton
      Left = 220
      Top = 8
      Width = 75
      Height = 25
      Caption = '&OK'
      ModalResult = 1
      TabOrder = 0
    end
    object CancelButton: TButton
      Left = 340
      Top = 8
      Width = 75
      Height = 25
      Caption = '&Cancel'
      ModalResult = 2
      TabOrder = 1
    end
  end
  object PeakColorScaleGroupBox: TGroupBox
    Left = 328
    Top = 24
    Width = 289
    Height = 297
    Caption = 'Peak Color Scale:'
    TabOrder = 3
    object PeakColorRedScaleGroupBox: TGroupBox
      Left = 8
      Top = 24
      Width = 273
      Height = 73
      Caption = 'Red Scale:'
      TabOrder = 0
      object PeakColorRedScaleTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 1000
        Orientation = trHorizontal
        PageSize = 50
        Frequency = 50
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = PeakColorRedScaleTrackBarChange
      end
      object PeakColorRedScaleWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = PeakColorRedScaleWidgetChange
      end
      object PeakColorRedScaleWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = PeakColorRedScaleWidget
        Min = 0
        Max = 1000
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
    object PeakColorGreenScaleGroupBox: TGroupBox
      Left = 8
      Top = 112
      Width = 273
      Height = 73
      Caption = 'Green Scale:'
      TabOrder = 1
      object PeakColorGreenScaleTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 1000
        Orientation = trHorizontal
        PageSize = 50
        Frequency = 50
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = PeakColorGreenScaleTrackBarChange
      end
      object PeakColorGreenScaleWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = PeakColorGreenScaleWidgetChange
      end
      object PeakColorGreenScaleWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = PeakColorGreenScaleWidget
        Min = 0
        Max = 1000
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
    object PeakColorBlueScaleGroupBox: TGroupBox
      Left = 8
      Top = 200
      Width = 273
      Height = 73
      Caption = 'Blue Scale:'
      TabOrder = 2
      object PeakColorBlueScaleTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 1000
        Orientation = trHorizontal
        PageSize = 50
        Frequency = 50
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = PeakColorBlueScaleTrackBarChange
      end
      object PeakColorBlueScaleWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = PeakColorBlueScaleWidgetChange
      end
      object PeakColorBlueScaleWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = PeakColorBlueScaleWidget
        Min = 0
        Max = 1000
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
  end
  object PeakColorOffsetGroupBox: TGroupBox
    Left = 328
    Top = 344
    Width = 289
    Height = 105
    Caption = 'Peak Color Offset:'
    TabOrder = 4
    object PeakColorOffsetColorContainerPanel: TPanel
      Left = 16
      Top = 24
      Width = 73
      Height = 73
      TabOrder = 0
      object PeakColorOffsetColorPanel: TPanel
        Left = 1
        Top = 1
        Width = 71
        Height = 71
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
      end
    end
    object PeakColorOffsetSelectButton: TButton
      Left = 112
      Top = 48
      Width = 75
      Height = 25
      Caption = 'Select...'
      TabOrder = 1
      OnClick = PeakColorOffsetSelectButtonClick
    end
  end
  object GlobalGroupBox: TGroupBox
    Left = 16
    Top = 472
    Width = 601
    Height = 105
    Caption = 'Global:'
    TabOrder = 5
    object GlobalScaleGroupBox: TGroupBox
      Left = 8
      Top = 16
      Width = 273
      Height = 73
      Caption = 'Scale:'
      TabOrder = 0
      object GlobalScaleTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 2000
        Orientation = trHorizontal
        PageSize = 100
        Frequency = 100
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = GlobalScaleTrackBarChange
      end
      object GlobalScaleWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = GlobalScaleWidgetChange
      end
      object GlobalScaleWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = GlobalScaleWidget
        Min = 0
        Max = 2000
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
    object GlobalOffsetGroupBox: TGroupBox
      Left = 320
      Top = 16
      Width = 273
      Height = 73
      Caption = 'Offset:'
      TabOrder = 1
      object GlobalOffsetTrackBar: TTrackBar
        Left = 24
        Top = 24
        Width = 150
        Height = 45
        LineSize = 10
        Max = 500
        Min = -500
        Orientation = trHorizontal
        PageSize = 50
        Frequency = 50
        Position = 0
        SelEnd = 0
        SelStart = 0
        TabOrder = 0
        TickMarks = tmBottomRight
        TickStyle = tsAuto
        OnChange = GlobalOffsetTrackBarChange
      end
      object GlobalOffsetWidget: TEdit
        Left = 184
        Top = 24
        Width = 57
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = GlobalOffsetWidgetChange
      end
      object GlobalOffsetWidgetUpDown: TUpDown
        Left = 241
        Top = 24
        Width = 15
        Height = 21
        Associate = GlobalOffsetWidget
        Min = -500
        Max = 500
        Position = 0
        TabOrder = 2
        Wrap = False
      end
    end
  end
  object BaseColorOffsetDialog: TColorDialog
    Ctl3D = True
    Options = [cdFullOpen]
    Left = 184
    Top = 408
  end
  object PeakColorOffsetDialog: TColorDialog
    Ctl3D = True
    Options = [cdFullOpen]
    Left = 496
    Top = 408
  end
end
