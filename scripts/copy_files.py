#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Dec 26 14:25:07 2018

@author: hippo
"""

import wx
import os.path
import shutil
import sys, traceback
#from shutil import copy2
import argparse
import time
import concurrent.futures

class CopyFrame(wx.Frame):

    def __init__(self, parent, title, source_paths, target_path):
        super(CopyFrame, self).__init__(parent, title=title,size=(350, 130),style=wx.DEFAULT_FRAME_STYLE ^ wx.RESIZE_BORDER)
        self.Centre()
        self.SetIcon(wx.Icon(getPicturePath('MrSoirIcon_cursor.png')))
        
        self.source_paths = source_paths
        self.target_path = target_path
        
        #panel = wx.Panel(self, wx.ID_ANY)
        
        vbox = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(vbox)
        
        self.gauge = wx.Gauge(self, range = 100, size = (250, 25), style= wx.GA_HORIZONTAL) 
        self.gauge.SetValue(30)
        vbox.Add(self.gauge, 0, wx.ALL|wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL, 5) 
        
        self.info_lbl = wx.StaticText(self,label = "copying files...", style=wx.GA_HORIZONTAL ) 
        vbox.Add(self.info_lbl, 0, wx.ALL|wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL, 5) 
        
        self.cncl_btn = wx.Button(self, label="cancel") 
        vbox.Add(self.cncl_btn, 0, wx.EXPAND)
        
        self.cncl_btn.Bind(wx.EVT_BUTTON, self.onCancelClicked)
        
        def startCopying(self):
        
    def updateGauge(self, value):
        self.gauge.SetValue(value)
        
    def incrementGauge(self):
        self.gauge.SetValue(self.gauge.GetValue() + 1)
        
    def startCopying(self):
        self.updateGauge(0)
        executor = concurrent.futures.ThreadPoolExecutor(max_workers=2)
        executor.submit(copyFiles, self.source_paths, self.target_path, self)
        
    def onCancelClicked(self, e):
        self._cancelled = True
        
    def cancelled(self):
        return self._cancelled

def print_test(sources, target, copy_frame):
    time.sleep(2)
    print('print_test... target: ', target)
    copy_frame.updateGauge(30)
    
    
def getPicturePath(fileName):
    gosh_base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
    return os.path.abspath(os.path.join(os.path.join(gosh_base_dir, "pics"), fileName))

def createUniqueFilePath(tar_path, file_name):
    cntr = 1
    name, ext = os.path.splitext(file_name)
    
    tar_file_path = os.path.join(tar_path, file_name)
    
    while os.path.exists(tar_file_path):
        tar_file_path = os.path.join(tar_path, name + "_(" + str(cntr) + ")" + ext)
        cntr += 1
    return tar_file_path

def copyFiles(source_paths, target_path, copy_dialog):
    try:
        if not os.path.exists(target_path):
            os.mkdir(target_path)
    except:
        print('error creating target_path')
        print("-"*60)
        traceback.print_exc()
        print("-"*60)
            
    for sp in source_paths:
        if copy_dialog.cancelled():
            return
        time.sleep(0.1)
        file_name = os.path.basename(sp)
        tar_entry_path = createUniqueFilePath(target_path, file_name)
        if os.path.isdir(sp) and not os.path.exists(tar_entry_path):
            try:
                os.mkdir(tar_entry_path)
                copy_dialog.incrementGauge()
                sub_entries = [os.path.join(sp, f) for f in os.listdir(sp)]
                copyFiles(sub_entries, tar_entry_path, copy_dialog)
            except:
                print('error while copying files')
                print("-"*60)
                traceback.print_exc()
                print("-"*60)
                copy_dialog.incrementGauge()
        else:
            try:
                shutil.copy2(sp, tar_entry_path)
                copy_dialog.incrementGauge()
            except:
                print('error while copying files')
                print("-"*60)
                traceback.print_exc()
                print("-"*60)
                copy_dialog.incrementGauge()
    copy_dialog.close

# /home/hippo/Documents/ -> /home/hippo/Documents -> eleminiert unnoetige path-separator am ende eines file-paths

def trimPathSeparator(pth):
    if pth[-1] is os.path.sep:
        return pth[:-1]
    return pth

def processPaths(paths):
    return [trimPathSeparator(p) for p in paths]

def isParentFolder(parent, sub):
    while len(sub) >= len(parent):
        if sub == parent:
            return True
        sub = os.path.dirname(sub)
    return False

def showWarningDialog(warning_msg):
    warning_dialog = wx.MessageDialog(None, warning_msg, style=wx.ICON_EXCLAMATION)
    warning_dialog.ShowModal()

def checkForSelfContaintingFiles(source_paths, tar_path):
    # checkt, dass ein source_path nicht teil eiens tar_paths ist. denn sonst 
    invalid_paths = set()
    tp = tar_path
    for s in source_paths:
        if isParentFolder(s, tp):
            chr_lmt = 200
            s_msg = "..." + s[-chr_lmt] if len(s) > chr_lmt else s
            t_msg = "..." + tar_path[-chr_lmt:] if len(tar_path) > chr_lmt else tar_path
            warning_msg = "You must not copy a parent folder into itself!\n " \
                        + '"' + s_msg + '"   ->    "' + t_msg + '"'
            showWarningDialog(warning_msg)
            invalid_paths.add(s)
    return source_paths - invalid_paths

def evalEntriesCount(source_pths):
    cntr = 0
    for sp in source_pths:
        if os.path.isdir(sp):
            cntr += 1
            sub_entries = [os.path.join(sp, f) for f in os.listdir(sp)]
            cntr += evalEntriesCount(sub_entries)
        else:
            cntr += 1
    return cntr

def main():
    parser = argparse.ArgumentParser()
#    parser.add_argument("-b", "--base_dir", help="base-target-directory: destination folder to copy files to")
    parser.add_argument("-s", "--sources", help="source paths of the files to copy", nargs="*", required=True)
    parser.add_argument("-t", "--target", help="target path", required=True)
    args = parser.parse_args()
    
    sources = set(processPaths(args.sources))
    target  = trimPathSeparator(args.target)
    
    app = wx.App()
    
    # parent-folder must not be copied into sub-folder
    sources = checkForSelfContaintingFiles(sources, target)
    
    entries_cnt = evalEntriesCount(sources)
    
    print('sources: ', sources)
    print('target: ', target)
    print('entries to copy: ', entries_cnt)
    
    cf = CopyFrame(None, title='Sizing', source_paths=sources, target_path=target)
    cf.gauge.SetRange(entries_cnt)
    cf.updateGauge(0)
    cf.Show()
            
    #copyFiles(sources, target, cf)
    #print('finished copying files!')
    

    app.MainLoop()


if __name__ == '__main__':
    main()