package com.thelogicmaster.crankedplugin

import com.intellij.openapi.util.Expirable
import com.intellij.util.containers.toArray
import com.intellij.xdebugger.XDebuggerUtil
import com.intellij.xdebugger.XExpression
import com.intellij.xdebugger.evaluation.EvaluationMode
import com.intellij.xdebugger.frame.XValue
import com.jetbrains.cidr.execution.debugger.CidrDebugProcess
import com.jetbrains.cidr.execution.debugger.CidrDebuggerUtil
import com.jetbrains.cidr.execution.debugger.CidrLocalDebugProcess
import com.jetbrains.cidr.execution.debugger.CidrStackFrame
import com.jetbrains.cidr.execution.debugger.actions.*
import com.jetbrains.cidr.execution.debugger.evaluate
import com.jetbrains.cidr.execution.debugger.evaluation.CidrEvaluatedValue
import com.jetbrains.cidr.execution.debugger.evaluation.CidrMemberValue
import com.jetbrains.cidr.execution.debugger.evaluation.CidrPhysicalValue
import com.jetbrains.cidr.execution.debugger.evaluation.CidrValue
import com.jetbrains.cidr.execution.debugger.memory.Address
import com.jetbrains.cidr.execution.debugger.memory.AddressRange
import java.awt.image.BufferedImage
import kotlin.experimental.and
import kotlin.math.ceil
import kotlin.math.roundToInt

class ImageDescriptor(val data: Array<Byte>, val width: Int, val height: Int, val stride: Int) : CidrImageDescriptor {
    override val shape: CidrImageShape
        get() = CidrImageShape.DEFAULT
    
    override val supportedFormats: List<CidrImageChannelFormat>
        get() = listOf(CidrImageChannelFormat.RGB)

    override fun getImage(format: CidrImageChannelFormat?): BufferedImage {
        val image = BufferedImage(width, height, BufferedImage.TYPE_INT_RGB)
        for (y in 0..<height)
            for (x in 0..<width) {
                val byte = data[y * stride + x / 8].toInt()
                val value = (byte and (1 shl (7 - x % 8))) > 0
                image.setRGB(x, y, if (value) (0xB0AEA7).toInt() else 0x302E27)
            }
        return image
    }

    override fun getPixelAddress(x: Int, y: Int): Address {
        return Address.NULL
    }

    override fun getPixelData(x: Int, y: Int): String? {
        return super.getPixelData(x, y)
    }
}

class ImageViewer(val imageExpr: String, val frame: CidrStackFrame) : CidrImageViewer {
    override suspend fun getImageDescriptor(debugProcess: CidrDebugProcess, expirable: Expirable, shape: CidrImageShape): CidrImageDescriptor {
        val bitmapExpr = imageExpr.replace("*", "") // This is awful, but allows viewing pointer types directly by evaluating `*image`
        val ctx = debugProcess.createEvaluationContext(debugProcess.driverInTests, null, frame) // Definitely fine to use the test accessor to the myDriverDoNotUse member
        val width = ctx.getData(ctx.evaluate("${bitmapExpr}.width")).intValue().toInt()
        val height = ctx.getData(ctx.evaluate("${bitmapExpr}.height")).intValue().toInt()
        val stride = ceil(width.toFloat() / 8).toInt()
        val buffer = ctx.getData(ctx.evaluate("${bitmapExpr}.data._M_impl._M_start")) // Super elegant and cross-platform way to access vector data
        val data = debugProcess.memory.hexdumpDoc.dataProvider.loadData(buffer.pointerAddress!!, AddressRange(buffer.pointerAddress!!, buffer.pointerAddress!! + stride * height)) // Seems suspect
        return ImageDescriptor(data[0].entries.map{ it.value }.toTypedArray(), width, height, stride)
    }

    override val useAsFullValueEvaluator: Boolean
        get() = true
}

class ImageProvider : CidrImageViewerProvider {
    override fun getImageViewer(value: XValue): CidrImageViewer? {
        if (value is CidrPhysicalValue) {
            if (value.type == "cranked::BitmapRef")
                return ImageViewer("(${value.`var`.referenceExpression}).resource", value.frame)
            else if (value.type == "cranked::LCDBitmap_32") // This doesn't really work, since the `View as image` prompt doesn't seem to show for pointer types
                return ImageViewer(value.`var`.referenceExpression, value.frame)
        }
        return null
    }
}
